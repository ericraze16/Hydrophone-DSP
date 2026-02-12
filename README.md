# Hydrophone-DSP

## Setting up Verilator 
We will use [verilator](https://github.com/verilator/verilator) here for 2 main reasons:
1. Faster hdl -> testing cycles
2. Easier to write test benches (write in cpp)

note: I (Eric) am using wsl

Installing Dependencies:

```bash
sudo apt-get update
sudo apt-get install -y git perl python3 make autoconf g++ flex bison \
    libgoogle-perftools-dev numactl perl-doc libfl2 libfl-dev zlibc zlib1g zlib1g-dev \
    gtkwave
```

Installing verilator from source:
```bash
git clone https://github.com/verilator/verilator
cd verilator
autoconf
./configure
make -j$(nproc)
sudo make install
```
