### Lua wrapper for C++ access similar to the Lua langauge itself.

[![Donate via Stripe](https://img.shields.io/badge/Donate-Stripe-green.svg)](https://buy.stripe.com/00gbJZ0OdcNs9zi288)<br>
[![Donate via Bitcoin](https://img.shields.io/badge/Donate-Bitcoin-green.svg)](bitcoin:37fsp7qQKU8XoHZGRQvVzQVP8FrEJ73cSJ)<br>

One part of this is a class wrapper to a Lua state.  From this there's index operators for quick Lua access by C++, using the global registry for refcounting.
Uses lua refs, detail classes, and smart pointers to let you create copy and destroy objects as you see fit without worrying about deallocation or order of lua stack operations or anything else. 

Another part of this is a class wrapper to the Lua stack, for just refcounting.

Another part is automatic Lua bindings.  Driven by a parallel Lua/C++ interface layer that needs to merged with the other two's...

TODO's for LuaCxx/Bind.h:
- needs member-method-returns-ref to work
- needs pass-by-value methods to push copies of full userdata instead of light userdata
- pairs for everyone + pairs-over-ipairs support for extra index / operator[] access objects
- expose C++ static members in the obj metatables (esp so the Lua metatable instances can access them, but so can the outside world via the metatable)
