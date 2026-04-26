# Nimonspoli CLI

Nimonspoli adalah proyek board game monopoli di CLI berbasis C++. 

## Struktur Proyek

```text
include/
  app/
  core/
    foundation/
    game_manager/
    services/
  models/
    base/
    cards/
    player/
    tiles/
  utils/
    data/
    io/

src/
  app/
  core/
    foundation/
    game_manager/
      commands/
    services/
  models/
    base/
    cards/
    player/
    tiles/
  utils/
    data/
    io/

config/
save/
tests/
```

## Build

`Makefile` proyek ini ditujukan untuk Linux / WSL.

Untuk build:

```bash
make
```

Hasil build akan berada di:

```text
bin/nimonspoli
```

Untuk membersihkan hasil build:

```bash
make clean
```

Target `clean` akan menghapus folder `build/`, `bin/`, dan `obj/`.

## Menjalankan Program

Setelah build selesai:

```bash
./bin/nimonspoli
```

Program akan meminta:
- direktori config
- `NEW` atau `LOAD`
- jumlah pemain dan nama pemain jika memilih game baru

Perilaku save/load:
- `LOAD` dilakukan saat program mulai
- jika prompt path load langsung di-`Enter`, program akan memakai alur default folder `save/`
- nama file save tanpa folder akan dianggap berada di dalam `save/`
- path eksplisit tetap bisa dipakai

## Command

Command runtime yang tersedia:

- `HELP`
- `CETAK_PAPAN`
- `CETAK_AKTA <kode>`
- `CETAK_PROPERTI`
- `LEMPAR_DADU`
- `ATUR_DADU <x> <y>`
- `NEXT`
- `BAYAR_DENDA`
- `GADAI <kode>`
- `TEBUS <kode>`
- `BANGUN <kode>`
- `GUNAKAN_KEMAMPUAN <idx>`
- `SIMPAN <path>`
- `CETAK_LOG [n]`

Catatan:
- `MUAT` tidak tersedia sebagai command in-game. Load dilakukan saat startup.
- `HELP` dipertahankan sebagai command bantuan.
- `NEXT` dipakai untuk melanjutkan atau mengakhiri state giliran setelah resolusi aksi.
- `BELI`, `LELANG`, pengumuman pemenang, dan pembuangan kartu saat tangan penuh berjalan otomatis, bukan command manual.
- `ATUR_DADU` hanya menerima nilai `1..6` untuk masing-masing dadu.

## Konfigurasi

Papan permainan dibangun dari file konfigurasi. File config core saat ini:

- `config/aksi.txt`
- `config/property.txt`
- `config/railroad.txt`
- `config/utility.txt`
- `config/tax.txt`
- `config/special.txt`
- `config/misc.txt`

`aksi.txt` diperlakukan sebagai file wajib untuk peletakan action tile pada papan 40 petak.

## Catatan

- Disarankan menjalankan proyek ini dari Linux atau WSL agar sesuai dengan ketentuan tugas.
- Folder `save/` ikut dilacak di repository, tetapi file save di dalamnya diabaikan oleh git.
