package main

import (
	"esp-bootloader/internal/configure"
	"esp-bootloader/internal/flash"
	"esp-bootloader/internal/shared"
	"flag"
	"fmt"
	"os"
	"slices"
	"strings"

	"github.com/k0kubun/pp/v3"
)

func main() {
	args := os.Args[1:]
	if len(args) == 0 {
		fmt.Fprintf(os.Stderr, "no arguments \n")
		fmt.Println("try --help or -h")
		os.Exit(1)
	} else {
		if args[0] == "--help" || args[0] == "-h" {
			fmt.Println("supported arguments: ")
			fmt.Println("operators: ", shared.Operators)
			fmt.Println("flags: ")
			fmt.Println("	-t (target): ", shared.Targets)
			fmt.Println("	-p (port): ", "/dev/...")
			fmt.Println("	-i (firmware images): ", "-i app1.bin -i  app2.bin...")
			return
		}
	}
	op := args[0]
	if !slices.Contains(shared.Operators, op) {
		fmt.Fprintf(os.Stderr, "unknown operator %s\n", op)
		fmt.Println("supported operators:", shared.Operators)
		os.Exit(1)
	}

	flags := flag.NewFlagSet(op, flag.ExitOnError)

	switch op {
	case shared.ConfigureOp:
		target := flags.String("t", "", "ESP target")
		flags.Parse(args[1:])
		if *target == "" || !slices.Contains(shared.Targets, *target) {
			fmt.Fprintf(os.Stderr, "unknown target %s\n", *target)
			fmt.Println("supported targets: ", shared.Targets)
			os.Exit(1)
		}
		if err := configure.ConfigureBoard(target); err != nil {
			fmt.Fprintf(os.Stderr, "configure error: %v\n", err)
			os.Exit(1)
		} else {
			fmt.Println("success")
			return
		}

	case shared.FlashOp:
		port := flags.String("p", "", "Flash port")
		target := flags.String("t", "", "ESP target")
		var images shared.StringSlice
		flags.Var(&images, "i", "Firmware images")

		flags.Parse(args[1:])

		if len(images) == 0 {
			fmt.Fprintf(os.Stderr, "empty app images \n")
			os.Exit(1)
		}
		if *port == "" {
			fmt.Fprintf(os.Stderr, "empty port \n")
			os.Exit(1)
		}
		if *target == "" || !slices.Contains(shared.Targets, *target) {
			fmt.Fprintf(os.Stderr, "unknown target %s\n", *target)
			fmt.Println("supported targets: ", shared.Targets)
			os.Exit(1)
		}

		part, err := flash.GetPartitionFile(shared.ProjectDir + "/partitions.csv")
		if err != nil {
			fmt.Fprintf(os.Stderr, "get partition error: %v\n", err)
			os.Exit(1)
		}

		apps, err := flash.GetApps(part)
		if err != nil {
			fmt.Fprintf(os.Stderr, "get apps error: %v\n", err)
			os.Exit(1)
		}

		match, notMatch := flash.GetMatchApps(apps, images)

		if len(match) > 0 {
			fmt.Println("Found matching and not matching firmware images:")
			for _, p := range match {
				pp.Println("Match", p.Image, "->", p.App)
			}
			if len(notMatch) > 0 {
				for _, p := range notMatch {
					pp.Println("Not match", p.Image, "->", p.App)
				}
			}

			fmt.Println("Flash matching images, then flash others? (Y/N)")

			var ans string
			fmt.Scan(&ans)

			if strings.ToLower(ans) == "y" {
				for _, p := range match {
					if err := flash.FlashApp(p, *target, *port); err != nil {
						pp.Errorf("flash app error: %v", err)
						os.Exit(1)
					}
				}
				for _, p := range notMatch {
					if err := flash.FlashApp(p, *target, *port); err != nil {
						pp.Errorf("flash app error: %v", err)
						os.Exit(1)
					}
				}
				fmt.Println("All flashed!")
				os.Exit(0)
			} else if strings.ToLower(ans) != "n" {
				fmt.Fprintln(os.Stderr, "unknown answer")
				os.Exit(1)
			}
		}

		for i, a := range apps {
			pp.Println(images[i], "->", a)
		}
		fmt.Println("Flash images by order? (Y/N)")
		var ans string
		fmt.Scan(&ans)

		if strings.ToLower(ans) == "y" {
			for i, image := range images {
				if i >= len(apps) {
					break
				}

				if err := flash.FlashApp(flash.Pair{App: apps[i], Image: image}, *target, *port); err != nil {
					pp.Errorf("flash app error: %v", err)
					os.Exit(1)
				}
			}
		} else if strings.ToLower(ans) == "n" {
			fmt.Println("Flashing cancelled")
		} else {
			fmt.Fprintln(os.Stderr, "unknown answer")
			os.Exit(1)
		}
	}
}
