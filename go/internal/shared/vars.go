package shared

import "strings"

const (
	ProjectDir    = "../"
	BuildDir      = ProjectDir + "build/"
	TargetsDir    = ProjectDir + "targets/"
	PartitionsDir = ProjectDir + "partitions/"

	SdkDefaultsFileName = "sdkconfig.defaults"
	ConfigureOp         = "configure"
	FlashOp             = "flash"

	FactoryName = "esp-bootloader"
)

var Operators = []string{
	ConfigureOp,
	FlashOp,
}

var Targets = []string{
	"esp32",
	"esp32s3",
	"esp32s2",
	"esp32c3",
	"esp32c6",
}

type StringSlice []string

func (s *StringSlice) String() string {
	return strings.Join(*s, ", ")
}

func (s *StringSlice) Set(value string) error {
	*s = append(*s, value)
	return nil
}
