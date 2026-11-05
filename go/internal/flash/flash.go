package flash

import (
	"bufio"
	"encoding/csv"
	"errors"
	"esp-bootloader/internal/shared"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

type App struct {
	Name   string
	Offset string
	Size   string
}

type Pair struct {
	App   App
	Image string
}

func GetPartitionFile(path string) ([]string, error) {
	if _, err := os.Stat(path); os.IsNotExist(err) {
		return []string{}, errors.New("no such file in directory")
	}
	f, err := os.Open(path)
	if err != nil {
		return nil, fmt.Errorf("open file: %w", err)
	}
	defer f.Close()
	p := make([]string, 0)
	input := bufio.NewScanner(f)
	for input.Scan() {
		p = append(p, input.Text())
	}
	if err = input.Err(); err != nil {
		return []string{}, err
	}
	return p, nil
}

func GetApps(partitions []string) ([]App, error) {
	reader := csv.NewReader(strings.NewReader(strings.Join(partitions, "\n")))
	reader.Comment = '#'
	reader.TrimLeadingSpace = true
	reader.FieldsPerRecord = -1

	records, err := reader.ReadAll()
	if err != nil {
		return nil, fmt.Errorf("parse partition CSV: %w", err)
	}

	apps := make([]App, 0)

	for i, col := range records {
		if len(col) < 5 {
			return nil, fmt.Errorf("invalid partition record at line %d", i+1)
		}

		if strings.TrimSpace(col[1]) != "app" || strings.TrimSpace(col[0]) == shared.FactoryName {
			continue
		}

		apps = append(apps, App{
			Name:   strings.TrimSpace(col[0]),
			Offset: strings.TrimSpace(col[3]),
			Size:   strings.TrimSpace(col[4]),
		})
	}

	return apps, nil
}

func GetMatchApps(apps []App, images []string) (match, notMatch []Pair) {
	byName := make(map[string]string, len(images))
	for _, image := range images {
		byName[strings.TrimSuffix(filepath.Base(image), filepath.Ext(image))] = image
	}

	used := make(map[string]bool, len(images))
	var freeApps []App
	for _, app := range apps {
		if image, ok := byName[app.Name]; ok {
			match = append(match, Pair{app, image})
			used[image] = true
		} else {
			freeApps = append(freeApps, app)
		}
	}

	var freeImages []string
	for _, image := range images {
		if !used[image] {
			freeImages = append(freeImages, image)
		}
	}

	for i := range min(len(freeApps), len(freeImages)) {
		notMatch = append(notMatch, Pair{freeApps[i], freeImages[i]})
	}
	return
}
func FlashApp(pair Pair, target, port string) error {
	cmd := exec.Command(
		"esptool.py",
		"--chip", target,
		"--port", port,
		"write_flash",
		pair.App.Offset,
		pair.Image,
	)
	cmd.Dir = shared.ProjectDir
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	fmt.Printf("Now flashing:%s\n...", pair.App.Name)
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("flash error%v", err)
	}
	return nil
}
