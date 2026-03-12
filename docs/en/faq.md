# FAQ

## Does Spike compile directly to machine code?

Not yet. The current backend emits C and then invokes a system C compiler.

## Why generate C first?

It reduces backend complexity and makes the project easier to learn.
