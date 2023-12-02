# GPT Partition Disk Shrink Utility

This is a very simple tool that shrinks unused space of a GPT disk. This maybe useful when you want to create a disk image from a relatively large harddrive and fit it into smaller drives.

## Build

This tool only requires `make`. It was developed under Ubuntu 22, but it should work out of the box for most unix systems.

```sh
$ make
```

## Example Usage

```sh
# replace "sda" with the disk or image file as you need

# Example 1: parse the gpt tables and exit
sudo gpt_shrink /dev/sda

# Example 2: shrink a disk and dump the binaries into a img file
sudo gpt_shrink /dev/sda > example.img

# Example 3: shrink a disk and directly flash into another disk
sudo gpt_shrink /dev/sda | sudo dd of=/dev/sdb
```

## License

This project is freely available in the public domain.
