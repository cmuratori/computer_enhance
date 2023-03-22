# Go Wrapper for 8086 Simulator

This Go wrapper for the 8086 Simulator is implemented using the `syscall`
package instead of `cgo`. The `cgo` approach has several limitations, such as the
need to specify `CFLAGS` and `LDFLAGS` in comments or environment variables, the
inability to use environment variables flexibly for specifying the DLL location,
and the requirement to include a header file (`sim86_shared.h`) that contains
incompatible syntax and data types.

By using the syscall package, this wrapper overcomes these issues and allows
for more flexibility when working with the 8086 Simulator Library.

# Building & Running

To run the program directly, execute the following command:
```
go run main.go sim86.go
```

To build an executable, use these commands:
```
go mod sim86
go build
```

This implementation has been tested on Go 1.20.2 for Windows/amd64.
