Общие команды для ядерных модулей:
```shell
modinfo hello-1.ko
```
```shell
lsmod | grep hello
```
```shell
sudo insmod hello-1.ko
```

The dash character will get converted to an underscore, so when you again try:
```shell
lsmod | grep hello
```
You should now see your loaded module. It can be removed again with:
```shell
sudo rmmod hello_1
```
Notice that the dash was replaced by an underscore. To see the module’s output messages, use dmesg to view the kernel log ring buffer:
```shell
sudo dmesg | tail -10
```
You should see messages like “Hello world 1.” and “Goodbye world 1.” from your module.
Alternatively, you can check the systemd journal for kernel messages:
```shell
journalctl --since "1 hour ago" | grep kernel
```

To view the output from your kernel modules, you must use `dmesg` or `journalctl -k `

insmod will fill the variables with any command line arguments that are given, like `insmod mymodule.ko myvariable=5`
The variable declarations and macros should be placed at the beginning of the module for clarity.
```shell
sudo insmod hello-5.ko mystring="bebop" myintarray=-1

sudo insmod hello-5.ko mystring="supercalifragilisticexpialidocious" myintarray=-1,-1

sudo insmod hello-5.ko mylong=hello
`insmod: ERROR: could not insert module hello-5.ko: Invalid parameters`
```

Obviously, we strongly suggest you to recompile your kernel,
so that you can enable a number of useful debugging features, such as forced module unloading ( MODULE_FORCE_UNLOAD ): 
when this option is enabled, 
you can force the kernel to unload a module even when it believes it is unsafe, via a `sudo rmmod -f module` command. 

To create a new char device named coffee with major/minor number 12 and 2, simply do
```shell
mknod /dev/coffee c 12 2 
```

