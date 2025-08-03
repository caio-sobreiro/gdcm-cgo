package main

import (
	"crypto/sha256"
	"fmt"
	"io"
	"log"
	"os"
	"path"
	"runtime"
	"strconv"
	"time"

	"gdcmconv-go/internal/services"
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

	inputDir := "input"
	outputCgoDir := "output-cgo"
	outputGdcmDir := "output-gdcm"

	files, err := os.ReadDir(inputDir)
	if err != nil {
		fmt.Println("Error reading input directory:", err)
		log.Fatal(err)
	}

	var inputFiles []string
	for _, file := range files {
		if !file.IsDir() {
			inputFiles = append(inputFiles, path.Join(inputDir, file.Name()))
		}
	}

	os.Mkdir(outputCgoDir, 0755)
	os.Mkdir(outputGdcmDir, 0755)

	fmt.Printf("Compressing %d files with %d concurrent goroutines using CGO...\n", len(inputFiles), concurrent)
	startCgo := time.Now()
	err = services.CompressSeries(inputFiles, outputCgoDir, concurrent)
	if err != nil {
		fmt.Println("Error:", err.Error())
	}
	timeCgo := time.Since(startCgo)
	fmt.Printf("Processing time: %v\n", timeCgo)

	// Compress series using the old method for comparison
	fmt.Printf("Compressing %d files with %d concurrent goroutines using gdcmconv...\n", len(inputFiles), concurrent)
	startGdcm := time.Now()
	err = services.CompressSeriesOld(inputFiles, outputGdcmDir, concurrent)
	if err != nil {
		fmt.Println("Error:", err.Error())
	}
	timeGdcm := time.Since(startGdcm)
	fmt.Printf("Processing time: %v\n", timeGdcm)

	gain := float64(timeGdcm.Nanoseconds()) / float64(timeCgo.Nanoseconds())
	fmt.Println()
	fmt.Printf("The CGO version is %.2f%% faster than the gdcmconv version.\n", (gain-1)*100)
	fmt.Println()
	fmt.Println("Current system specs:")
	fmt.Printf("OS: %s, Architecture: %s, Go Version: %s\n", runtime.GOOS, runtime.GOARCH, runtime.Version())

	// proof that the output files are the same
	fmt.Println()
	fmt.Println("Proof that the output files are the same:")
	fmt.Printf("Comparing files in %s and %s by sha256 hash...\n", outputCgoDir, outputGdcmDir)
	allMatch := true
	for _, file := range inputFiles {
		hashCgo, _ := hashFile(path.Join(outputCgoDir, path.Base(file)))
		hashGdcm, _ := hashFile(path.Join(outputGdcmDir, path.Base(file)))
		if hashCgo != hashGdcm {
			fmt.Printf("File %s differs between CGO and gdcmconv outputs\n", path.Base(file))
			allMatch = false
		}
	}

	if allMatch {
		fmt.Println("All files match between CGO and gdcmconv outputs.")
	} else {
		fmt.Println("Some files do not match between CGO and gdcmconv outputs.")
	}
	fmt.Println("Finished comparing outputs.")
}

/* Hash file using sha256*/
func hashFile(filePath string) (string, error) {
	file, err := os.Open(filePath)
	if err != nil {
		return "", err
	}
	defer file.Close()

	hasher := sha256.New()
	if _, err := io.Copy(hasher, file); err != nil {
		return "", err
	}

	hash := hasher.Sum(nil)
	return fmt.Sprintf("%x", hash), nil
}
