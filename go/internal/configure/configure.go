package configure

import (
	"esp-bootloader/internal/shared"
	"fmt"
	"os"
	"os/exec"
)

func configureSkd(target *string) error {
	if err := shared.CopyFile(shared.TargetsDir+*target+"/"+shared.SdkDefaultsFileName, shared.ProjectDir+shared.SdkDefaultsFileName); err != nil {
		return err
	}

	if err := os.Remove(shared.ProjectDir + "sdkconfig"); err != nil && !os.IsNotExist(err) {
		return err
	}

	cmd := exec.Command(
		"idf.py",
		"-D",
		"SDKCONFIG_DEFAULTS="+shared.SdkDefaultsFileName,
		"reconfigure",
	)

	cmd.Dir = shared.ProjectDir
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("configure target %s: %w", *target, err)
	}
	return nil
}


func ConfigureBoard(target *string) error {
	if *target == "" {
		return fmt.Errorf("empty target")
	}
	i, err := shared.CheckTarget(*target)
	if err != nil {
		return err
	}

	*target = shared.Targets[i]
	if err := configureSkd(target); err != nil {
		return fmt.Errorf("sdk configure error: %v", err)
	}
	return nil
}
