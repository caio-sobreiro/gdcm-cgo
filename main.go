package main

import (
	"fmt"
	"os"
	"strconv"
	"time"

	"go-dicom-conv/services"
)

func main() {
	var concurrent int
	if len(os.Args) > 1 {
		var err error
		concurrent, err = strconv.Atoi(os.Args[1])
		if err != nil {
			fmt.Println("Invalid concurrency value, using default 10")
			concurrent = 10
		}
	} else {
		concurrent = 10 // default concurrency level
	}
	fmt.Printf("Starting compression with %d concurrent goroutines...\n", concurrent)

	start := time.Now()
	err := services.CompressSeries("input", "output", concurrent)
	if err != nil {
		fmt.Println("Error:", err.Error())
	}
	fmt.Printf("Processing time: %v\n", time.Since(start))
}
