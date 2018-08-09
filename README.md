# FastSignals

C++17 signals/slots implementation with Boost.Signals2 compatible API.

>If you are not familar with Boost.Signals2, please read [Boost.Signals2: Connections](https://theboostcpplibraries.com/boost.signals2-connections)

Pros:

* Faster than Boost.Signals2
* Has more compact binary code
* Has the same API as Boost.Signals2

Cons:

* Supports only C++17 compatible compilers: Visual Studio 2017, modern Clang, modern GCC
* Lacks a few rarely used features presented in Boost.Signals2
    * No access to connection from slot with `signal::connect_extended` method
    * No connected object tracking with `slot::track` method
    * No temporary signal blocking with `shared_connection_block` class
    * Cannot disconnect equivalent slots since no `disconnect(slot)` function overload
    * Any other API difference is a bug - please report it!

## Examples

### Example with signal&lt;&gt; and connection

```cpp
// Creates signal and connects 1 slot, calls 2 times, disconnects, calls again.
// Outputs:
//  13
//  17
#include "libfastsignals/signal.h"

using namespace is::signals;

int main()
{
    signal<void(int)> valueChanged;
    connection conn;
    conn = valueChanged.connect([](int value) {
        cout << value << endl;
    });
    valueChanged(13);
    valueChanged(17);
    conn.disconnect();
    valueChanged(42);
}
```

### Example with scoped_connection

```cpp
// Creates signal and connects 1 slot, calls 2 times, calls again after scoped_connection destroyed.
//  - note: scoped_connection closes connection in destructor
// Outputs:
//  13
//  17
#include "libfastsignals/signal.h"

using namespace is::signals;

int main()
{
    signal<void(int)> valueChanged;
    {
        scoped_connection conn;
        conn = valueChanged.connect([](int value) {
            cout << value << endl;
        });
        valueChanged(13);
        valueChanged(17);
    }
    valueChanged(42);
}
```

## Benchmark results

Directory `tests/libfastsignals_bench` contains simple benchmark with compares two signal/slot implementations:

* Boost.Signals2
* libfastsignals

Benchmark compairs performance when signal emitted frequently with 0, 1 and 8 active connections. In these cases libfastsignals is 3-6 times faster.

```
*** Results:
measure                 emit_boost   emit_fastsignals
emit_boost/0                  1.00               3.00
emit_boost/1                  1.00               5.76
emit_boost/8                  1.00               3.70
***
```
