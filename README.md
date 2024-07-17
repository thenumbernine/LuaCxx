### Lua wrapper for C++ access similar to the Lua langauge itself.

[![Donate via Stripe](https://img.shields.io/badge/Donate-Stripe-green.svg)](https://buy.stripe.com/00gbJZ0OdcNs9zi288)<br>

One part of this is a class wrapper to a Lua state.  From this there's index operators for quick Lua access by C++, using the global registry for refcounting.
Uses lua refs, detail classes, and smart pointers to let you create copy and destroy objects as you see fit without worrying about deallocation or order of lua stack operations or anything else. 

Another part of this is a class wrapper to the Lua stack, for just refcounting.

Another part is automatic Lua bindings.  Driven by a parallel Lua/C++ interface layer that needs to merged with the other two's...

TODO's for LuaCxx/Bind.h:
- pairs for everyone + pairs-over-ipairs support for extra index / operator[] access objects
