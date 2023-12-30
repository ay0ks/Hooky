# Hooky
-----
Install:
```
make && sudo make install
```

Simple usage:
```py
from Hooky import hook, hook_member, unhook

@hook(str)
def test(self, *args, **kwargs):
    print(self, args, kwargs)

str.test()
"123".test()
    
hook_member(int, "hooky_real", 123)
print(int.hooky_real, (0).hooky_real)

unhook(str)
unhook(int)
```
