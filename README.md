# GDCM CGO implementation
This is a proof-of-concept project that implements a wrapper on the C++ GDCM library for transcoding a DICOM image into JPEG2000.

A commonly used alternative is to invoke the gdcmconv CLI tool directly (using https://pkg.go.dev/os/exec or https://nodejs.org/api/child_process.html), but the CGO approach has proven to be around 25% faster (see below).

## Performance comparison
![CGO vs External process performance comparison](CGO%20vs%20External%20process%20%28Node.js%29.png)

## Running the project
1- Compile the C++ GDCM wrapper
Note the arm64 architecture - adjust if necessary
```bash
cd cgdcm
g++ -std=c++11 -arch arm64 -I/usr/local/include/gdcm-3.3 -c gdcmconv.cpp -o gdcmconv.o
```

2- Add your DICOM files to the `input` folder (create one if it doesn't exist), then run:

```bash
go run main.go [concurrency]
```
or
```bash
go build -o go-gdcm .
./go-gdcm [concurrency]
```
