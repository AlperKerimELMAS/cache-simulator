# cache-simulator

A cache simulator written in C. It simulates two cache levels: separate L1
instruction and data caches (L1I and L1D) and a shared L2. It reads a memory
trace, prints the hit/miss result of each access for every level, and gives
hit, miss and eviction counts at the end.

The caches are set-associative and use FIFO replacement. Stores are
write-through and a store miss does not bring the block into the cache. Main
memory is 16 MB and can be loaded from a file at startup.

I'm not developing this project any further.

## Building

You need gcc and make.

```
make
```

This builds `cache_simulator` in the project folder. `make clean` removes it
and the object files.

## Running

```
./cache_simulator -L1s <s> -L1E <E> -L1b <b> -L2s <s> -L2E <E> -L2b <b> -t <trace>
```

- `s` is the number of set index bits (the cache has 2^s sets)
- `E` is the number of lines per set
- `b` is the number of block offset bits (blocks are 2^b bytes)
- `-t` is the name of the trace file

The L1 options are used for both L1I and L1D.

The program looks for its input files in a `data` folder in the current
directory, so run it from the project folder:

```
data/RAM.dat            initial memory contents, raw binary (optional)
data/traces/<trace>     the trace file passed with -t
```

`data/` is in .gitignore so you have to create it yourself. If there is no
RAM.dat, memory starts as all zeros.

## Trace format

Each line is one memory access. Addresses and store data are in hex, sizes
are in bytes.

```
I 0, 4
L 10, 4
S 10, 2, abcd
M 20, 1, ff
L 10, 4
```

- `I addr, size` loads an instruction (L1I and L2)
- `L addr, size` loads data (L1D and L2)
- `S addr, size, data` stores data
- `M addr, size, data` is a load followed by a store to the same address

For loads the size isn't used since the whole block is fetched.

## Output

Running the trace above with
`-L1s 0 -L1E 2 -L1b 3 -L2s 1 -L2E 2 -L2b 3 -t example.trace` gives:

```
RAM.dat could not be found. No memory load performed!
I 0, 4
 L1I miss, L2 miss
 Place in L2 set 0, L1I
L 10, 4
 L1D miss, L2 miss
 Place in L2 set 0, L1D
S 10, 2, abcd
 L1D hit, L2 hit
 Store in L1D L2 RAM
M 20, 1, ff
 L1D miss, L2 miss
 Place in L2 set 0, L1D
 L1D hit, L2 hit
 Store in L1D L2 RAM
L 10, 4
 L1D hit, L2 hit

L1I-hits:0 L1I-misses:1 L1I-evictions:0
L1D-hits:3 L1D-misses:2 L1D-evictions:0
L2-hits:3 L2-misses:3 L2-evictions:1
```

## Limitations

- L2 is checked on every access, including when L1 hits, so the L2 counts
  include those accesses too.
- L1 and L2 should have the same block size. The block is placed in L2 using
  the L1 block address, so an L2 block size smaller than L1 gives wrong
  results.
- There is almost no input checking. Options that aren't given default to 0,
  and E = 0 or an option without a value will crash the program.
- Buffers have fixed sizes: addresses have to be inside the 16 MB memory,
  stores can be up to 31 bytes, block sizes up to 256 bytes (b <= 8), and the
  trace file name has to be shorter than 40 characters.

## License

Mozilla Public License 2.0, see [LICENSE](LICENSE).
