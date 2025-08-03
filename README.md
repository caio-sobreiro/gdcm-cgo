# POC
This project is a proof-of-concept (POC) for transcoding DICOM images to JPEG2000 using the GDCM library in Go.
I have started an actual library implementation at https://github.com/caio-sobreiro/gdcmconv-go that should eventually include all of the functionality of the gdcmconv tool.

# GDCM CGO implementation
This is a proof-of-concept project that implements a wrapper on the C++ GDCM library for transcoding a DICOM image into JPEG2000.

A commonly used alternative is to invoke the gdcmconv CLI tool directly (using https://pkg.go.dev/os/exec), but the CGO approach has proven to be around 25-30% faster (see below).

Calling an external process means incurring system call overhead, which is not negligible, especially when processing a large number of files. The CGO approach allows for more efficient memory management and better performance.

For more information on CGO, see https://pkg.go.dev/cmd/cgo.

## Performance comparison
![CGO vs External process performance comparison](CGO%20vs%20External%20process%20%28Node.js%29.png)

## Running the project
1- Add your DICOM files to the `input` folder (create one if it doesn't exist)

2- Run:
```bash
go run cmd/gdcm-go/main.go [concurrency]
```
or
```bash
go build -o gdcm-go cmd/gdcm-go/main.go
./gdcm-go [concurrency]
```
