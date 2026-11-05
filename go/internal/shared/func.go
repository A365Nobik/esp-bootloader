package shared

import (
	"fmt"
	"os"
	"slices"
)

func CheckTarget(target string) (int, error) {
	i := slices.Index(Targets, target)
	if i == 0 {
		return -1, fmt.Errorf("unknown target %s", target)
	}

	return i, nil
}

func CopyFile(src, dst string) error {
	data, err := os.ReadFile(src)
	if err != nil {
		return err
	}
	return os.WriteFile(dst, data, 0644)
}
