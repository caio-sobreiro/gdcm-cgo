package services

/*
#cgo CXXFLAGS: -std=c++11 -I/usr/local/include/gdcm-3.3
#cgo LDFLAGS: -L/usr/local/lib -lgdcmMSFF -lgdcmDICT -lgdcmCommon -lexpat -lstdc++ -lgdcmDSED cgdcm/gdcmconv.o

#include <stdlib.h>

extern int convert_to_jpeg2000(const char* input, const char* output);
*/
import "C"

import (
	"fmt"
	"os"
	"unsafe"
)

func ConvertToJPEG2000(inputPath, outputPath string) error {
	inputPathPtr := C.CString(inputPath)
	outputPathPtr := C.CString(outputPath)

	defer C.free(unsafe.Pointer(inputPathPtr))
	defer C.free(unsafe.Pointer(outputPathPtr))

	result := C.convert_to_jpeg2000(inputPathPtr, outputPathPtr)
	if result != 0 {
		return fmt.Errorf("failed to convert DICOM file to JPEG2000.")
	}
	return nil
}

func CompressSeries(inputPath string, outputPath string, concurrent int) error {
	files, err := os.ReadDir(inputPath)
	if err != nil {
		fmt.Println("Error reading input directory:", err)
		return err
	}

	// create goroutine pool for concurrent processing
	pool := make(chan struct{}, concurrent) // limit to N concurrent goroutines
	defer close(pool)

	for _, file := range files {
		inputPath := "input/" + file.Name()
		outputPath := "output/" + file.Name()

		pool <- struct{}{} // acquire a slot in the pool
		go func(inputPath, outputPath string) {
			defer func() { <-pool }() // release the slot when done
			if err := ConvertToJPEG2000(inputPath, outputPath); err != nil {
				fmt.Println("Error converting file:", err)
			} else {
				fmt.Println("Successfully converted:", outputPath)
			}
		}(inputPath, outputPath)
	}

	// Wait for all goroutines to finish
	for i := 0; i < cap(pool); i++ {
		pool <- struct{}{}
	}
	return nil
}
