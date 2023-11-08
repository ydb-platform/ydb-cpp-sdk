package main

import (
	"bufio"
	"flag"
	"fmt"
	"io"
	"os"
	"strings"
)

func fatalf(msg string, args ...interface{}) {
	_, _ = fmt.Fprintf(os.Stderr, msg+"\n", args...)
	os.Exit(1)
}

func generate(w io.Writer, pkg string, blobs [][]byte, keys []string) {
	_, _ = fmt.Fprint(w, "// Code generated by github.com/ydb-platform/ydb/library/go/core/resource/cc DO NOT EDIT.\n")
	_, _ = fmt.Fprintf(w, "package %s\n\n", pkg)
	_, _ = fmt.Fprint(w, "import \"github.com/ydb-platform/ydb/library/go/core/resource\"\n")

	for i := 0; i < len(blobs); i++ {
		blob := blobs[i]

		_, _ = fmt.Fprint(w, "\nfunc init() {\n")

		_, _ = fmt.Fprint(w, "\tblob := []byte(")
		_, _ = fmt.Fprintf(w, "%+q", blob)
		_, _ = fmt.Fprint(w, ")\n")
		_, _ = fmt.Fprintf(w, "\tresource.InternalRegister(%q, blob)\n", keys[i])
		_, _ = fmt.Fprint(w, "}\n")
	}
}

func main() {
	var pkg, output string

	flag.StringVar(&pkg, "package", "", "package name")
	flag.StringVar(&output, "o", "", "output filename")
	flag.Parse()

	if flag.NArg()%2 != 0 {
		fatalf("cc: must provide even number of arguments")
	}

	var keys []string
	var blobs [][]byte
	for i := 0; 2*i < flag.NArg(); i++ {
		file := flag.Arg(2 * i)
		key := flag.Arg(2*i + 1)

		if !strings.HasPrefix(key, "notafile") {
			fatalf("cc: key argument must start with \"notafile\" string")
		}
		key = key[8:]

		if file == "-" {
			parts := strings.SplitN(key, "=", 2)
			if len(parts) != 2 {
				fatalf("cc: invalid key syntax: %q", key)
			}

			keys = append(keys, parts[0])
			blobs = append(blobs, []byte(parts[1]))
		} else {
			blob, err := os.ReadFile(file)
			if err != nil {
				fatalf("cc: %v", err)
			}

			keys = append(keys, key)
			blobs = append(blobs, blob)
		}
	}

	f, err := os.Create(output)
	if err != nil {
		fatalf("cc: %v", err)
	}

	b := bufio.NewWriter(f)
	generate(b, pkg, blobs, keys)

	if err = b.Flush(); err != nil {
		fatalf("cc: %v", err)
	}

	if err = f.Close(); err != nil {
		fatalf("cc: %v", err)
	}
}
