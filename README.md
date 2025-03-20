# Judger 

A sandbox to securely execute untrusted programs in the judging system

## How to build `libjudger.so`

1. Reopen in container  
2. Execute the following command in the terminal  

```bash
./build.sh
```

You can find the `libjudger.so` in the `output` directory.  

## How to use `libjudger.so`

1. Reopen in container (after enter the container, run `entrypoint.sh`)
2. Build the `libjudger.so` and a test program  

`Main.java`:  

```java
import java.util.Scanner;

public class Main {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int a = sc.nextInt();
        int b = sc.nextInt();
        System.out.println(a + b);
    }
}
```  

`input.txt`:  

```plaintext
1 2
```

Run the following command to compile the test program  

```bash
javac Main.java
```
3. Execute the following command in the terminal  

```bash
sudo -E ./output/libjudger.so --max_memory=100000 --exe_path="/usr/bin/java" --args="Main" --input_path='input.txt' --output_path='output.txt'
```

You can check the OOM killer has been triggered by the following command  

```bash
cd /sys/fs/cgroup/sandbox-${CONTAINER_ID}
cat memory.events
# output
low 0
high 0
max 7335
oom 1
oom_kill 1
oom_group_kill 0
```

## What methodological changes have been made

For the original judger, the sandbox is implemented by `setrlimit`.  
However, the `setrlimit` method is not precise enough to limit the memory usage of the program.  
Therefore, we use `cgroup` to limit the memory usage of the program.  
