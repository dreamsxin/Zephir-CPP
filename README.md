Zephir-CPP
==========

Zephir-CPP is c++ implementation of the zephir.

# Requirements

```shell
sudo apt-get install libboost1.54-dev
sudo apt-get install libboost-program-options1.55-dev
sudo apt-get install libboost-system1.55-dev
sudo apt-get install libboost-filesystem1.55-dev
sudo apt-get install libboost-regex1.55-dev
```

# Tests
```shell
valgrind --tool=memcheck --leak-check=full ./bin/zephir-cpp --run unit-tests/hello.zep
```
hello.zep
```shell
string ret, message = "hello";
int size;

echo message;

let message = message + " world!";

echo message;

let size = message->length();

echo "size is " + size;

if size > 0 {
	echo "size is greater zero";
	while size > 0 {
		let --size;
		if size % 2 == 0 {
			echo "size%2 == 0";
			continue;
		}
		echo "size is " + size;
	}
}

echo say("Zephir");

/**
 * Test function
 */
string function say(var str) {
	return "My name is " + str;
}
```