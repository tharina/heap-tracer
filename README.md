# Heap tracer


```
gcc -o hook.so -fPIC -shared hook.c -ldl
```

Run the executable with: `LD_PRELOAD=PATH/TO//hook.so ./myexecutable`

or in GDB with: `set exec-wrapper env 'LD_PRELOAD=./hook.so'`
