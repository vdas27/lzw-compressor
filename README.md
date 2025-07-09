Command line LZW file compressor built using C. Project completed November 2024.
## Prerequisites
* GCC compiler
* The Standard C Library
## Usage
Navigate to the src directory.<br>
Run `make`
### Compression
`cat file_name | ./encode -m (integer between 9 and 20) -p (if pruning wanted) >> compression_file`
- -m (optional)
  - choose number of bits printed per entry. Default set to 12
- -p (optional)
  - Enable table pruning
### Decompression
`cat compression_file | ./decode >> output_file`
## Features
**Pruning:** In the original algorithm, when the table reaches max additional entries, no new entries are added from that point on.
Pruning allows the table to be dynamically resized by removing non frequent substrings from the table when the table is full.
Entries which are not used as prefixes for other entries are removed from the table, allowing for new entries to be added<br>
**Bit Packing:** By default, the algorithm uses 12 bits to represent the dictionary entries.
As a consequence, the maximum dictionary size is 4096 entries (2^12). Implemented a feature that allows the amount of bits 
used to represent the dictionary entries to be a user input. The program supports from 8 to 20 bits per entry. This allows 
for finer control over the output. Additionally, to account for larger entry bit sizes potentially taking up more space,
the program only will print out as many bits as is needed to print out the current maximum dictionary size. For example,
if the user has set the the maximum bits per entry to 12, but the dictionary currently has 4096 entries, the program will only
print out 9 bits per entry. However, once the dictionary exceeds 4096 entries, it will then start printing out 10 bits per entry,
and will continue this trend until 12 bits per entry is required to print dicitionary entries.<br>
**Print Dictionary Table:** To view the dictionary tables created during compression and decompression, set environment variable `DBG` to 1.
## Design Process
Development process and design challenges are journaled in the logfiles directory
