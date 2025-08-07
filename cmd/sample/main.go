package main

import (
	gdcmconv "github.com/caio-sobreiro/gdcmconv-go"
)

func main() {
	gdcmconv.Convert("input.dcm", "output.dcm")
}
