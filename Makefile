#
# @eosfomo3d
#

CONTRACT = eosfomo3d

all: abi wast


abi:
	eosiocpp -g $(CONTRACT).abi $(CONTRACT).hpp

wast:
	eosiocpp -o $(CONTRACT).wast $(CONTRACT).cpp

clean:
	rm -rf $(CONTRACT).abi $(CONTRACT).wasm $(CONTRACT).wast

.PHONY: all abi wast clean
