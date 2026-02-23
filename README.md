# tm

A simple task manager.

- `make`
- `gcc`

```sh
make
./tm
cd project_dir
./tm init
./tm add "Here is the title"
./tm list

sudo make install
tm
```

For easy integration with other tools, here is the output format of `tm add`:

```
Task created: <title>
<timestamp>
<file_path>:2: <title>
```
