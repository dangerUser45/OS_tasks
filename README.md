# 📘 Информатика — 3 семестр  

[![Language: C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![OS: Linux](https://img.shields.io/badge/OS-Linux-green.svg)](https://kernel.org)
![Semester](https://img.shields.io/badge/Semester-3rd-orange)
[![Repo](https://img.shields.io/badge/GitHub-OS__tasks-black?logo=github)](https://github.com/dangerUser45/OS_tasks)

Семинарские работы по системному программированию на языке **C**.  
Каждый семинар посвящён отдельным механизмам Linux: процессы, каналы, IPC, семафоры, mmap, pthreads, файловые системы и сигналы.

---

# 🖼️ Превью

> ⚠️ *Здесь временные заглушки — ты можешь позже заменить их на реальные скриншоты вывода программ.*

### Пример: вывод `my_ls`
![preview ls](https://via.placeholder.com/700x250.png?text=Preview%3A+my_ls+output)

### Пример: работа shell
![preview shell](https://via.placeholder.com/700x250.png?text=Preview%3A+my_shell+session)

---

# ▶️ Как запускать

Каждый семинар имеет собственный скрипт сборки:

```
./build.sh
```

После сборки исполняемые файлы находятся в каталоге:

```
<seminar>/build/
```

Примеры:

```bash
cd 4th_seminar
./build.sh
./build/my_shell
```

```bash
cd 8th_seminar
./build.sh
./build/pthread_pizza.out 3 3 5
```

---

# 📑 Оглавление
- [📘 Информатика — 3 семестр](#-информатика--3-семестр)
- [🖼️ Превью](#️-превью)
    - [Пример: вывод `my_ls`](#пример-вывод-my_ls)
    - [Пример: работа shell](#пример-работа-shell)
- [▶️ Как запускать](#️-как-запускать)
- [📑 Оглавление](#-оглавление)
- [1-й семинар](#1-й-семинар)
- [2-й семинар](#2-й-семинар)
    - [Файлы и задачи:](#файлы-и-задачи)
- [3-й семинар](#3-й-семинар)
    - [Файлы:](#файлы)
- [4-й семинар](#4-й-семинар)
    - [Файлы:](#файлы-1)
- [5-й семинар](#5-й-семинар)
    - [Файлы:](#файлы-2)
- [6-й семинар](#6-й-семинар)
    - [Файлы:](#файлы-3)
- [7-й семинар](#7-й-семинар)
    - [Файлы:](#файлы-4)
- [8-й семинар](#8-й-семинар)
    - [Файл:](#файл)
- [9-й семинар](#9-й-семинар)
- [10-й семинар](#10-й-семинар)
    - [Файл:](#файл-1)
- [11-й семинар](#11-й-семинар)
    - [Файлы:](#файлы-5)
- [12-й семинар](#12-й-семинар)
    - [Файл:](#файл-2)
- [Библиотека safe\_lib](#библиотека-safe_lib)

---

# 1-й семинар  
📂 **[1st_seminar](https://github.com/dangerUser45/OS_tasks/tree/main/1st_seminar)**  

Тема: **аналог команды `echo`**

Задача: вывести аргументы командной строки.

---

# 2-й семинар  
📂 **[2nd_seminar](https://github.com/dangerUser45/OS_tasks/tree/main/2nd_seminar/src)**  

Тема: **fork() и создание процессов**  

### Файлы и задачи:
- [`exec.c`](https://github.com/dangerUser45/OS_tasks/blob/main/2nd_seminar/src/exec.c) — работа `exec*()`.  
- [`fork_parallel.c`](https://github.com/dangerUser45/OS_tasks/blob/main/2nd_seminar/src/fork_parallel.c) — параллельные процессы.  
- [`fork_series.c`](https://github.com/dangerUser45/OS_tasks/blob/main/2nd_seminar/src/fork_series.c) — процессы по очереди + `wait()`.  
- [`sleepsort.c`](https://github.com/dangerUser45/OS_tasks/blob/main/2nd_seminar/src/sleepsort.c) — сортировка через sleep.

---

# 3-й семинар  
📂 **[3rd_seminar](https://github.com/dangerUser45/OS_tasks/tree/main/3rd_seminar/src)**  

Тема: **низкоуровневая работа с файлами**

### Файлы:
- [`my_cat.c`](https://github.com/dangerUser45/OS_tasks/blob/main/3rd_seminar/src/my_cat.c) — `cat`.  
- [`my_cp.c`](https://github.com/dangerUser45/OS_tasks/blob/main/3rd_seminar/src/my_cp.c) — `cp`.

---

# 4-й семинар  
📂 **[4th_seminar](https://github.com/dangerUser45/OS_tasks/tree/main/4th_seminar/src)**  

Тема: **конвейеры: pipe(), dup2() и простой shell**

### Файлы:
- [`my_shell.c`](https://github.com/dangerUser45/OS_tasks/blob/main/4th_seminar/src/my_shell.c)  
- [`shell_parser.c`](https://github.com/dangerUser45/OS_tasks/blob/main/4th_seminar/src/shell_parser.c)  
- [`my_wc.c`](https://github.com/dangerUser45/OS_tasks/blob/main/4th_seminar/src/my_wc.c)  
- [`pipe_cat.c`](https://github.com/dangerUser45/OS_tasks/blob/main/4th_seminar/src/pipe_cat.c)

---

# 5-й семинар  
📂 **[5th_seminar](https://github.com/dangerUser45/OS_tasks/tree/main/5th_seminar/src)**  

Тема: **очереди сообщений (System V & POSIX MQ)**

### Файлы:
- [`att_relay_race.c`](https://github.com/dangerUser45/OS_tasks/blob/main/5th_seminar/src/att_relay_race.c)  
- [`posix_relay_race.c`](https://github.com/dangerUser45/OS_tasks/blob/main/5th_seminar/src/posix_relay_race.c)

---

# 6-й семинар  
📂 **[6th_seminar](https://github.com/dangerUser45/OS_tasks/tree/main/6th_seminar/src)**  

Тема: **System V IPC — семафоры**

### Файлы:
- [`shower.c`](https://github.com/dangerUser45/OS_tasks/blob/main/6th_seminar/src/shower.c)  
- [`shower_opt.c`](https://github.com/dangerUser45/OS_tasks/blob/main/6th_seminar/src/shower_opt.c)

---

# 7-й семинар  
📂 **[7th_seminar](https://github.com/dangerUser45/OS_tasks/tree/main/7th_seminar/src)**  

Тема:  
- POSIX семафоры  
- `mmap()`  
- POSIX shared memory

### Файлы:
- [`mmap_cp.c`](https://github.com/dangerUser45/OS_tasks/blob/main/7th_seminar/src/mmap_cp.c)  
- [`pizza.c`](https://github.com/dangerUser45/OS_tasks/blob/main/7th_seminar/src/pizza.c)

---

# 8-й семинар  
📂 **[8th_seminar](https://github.com/dangerUser45/OS_tasks/tree/main/8th_seminar/src)**  

Тема: **POSIX threads**

### Файл:
- [`pthread_pizza.c`](https://github.com/dangerUser45/OS_tasks/blob/main/8th_seminar/src/pthread_pizza.c)

---

# 9-й семинар  
📂 **[9th_seminar](https://github.com/dangerUser45/OS_tasks/tree/main/9th_seminar/src)**  

Тема: **контрольная работа по семафорам и shared memory**  

---

# 10-й семинар  
📂 **[10th_seminar](https://github.com/dangerUser45/OS_tasks/tree/main/10th_seminar/src)**  

Тема: **файловые системы: opendir(), readdir()**

### Файл:
- [`my_ls.c`](https://github.com/dangerUser45/OS_tasks/blob/main/10th_seminar/src/my_ls.c)

---

# 11-й семинар  
📂 **[11th_seminar](https://github.com/dangerUser45/OS_tasks/tree/main/11th_seminar/src)**  

Тема: **сигналы и их безопасная обработка**

### Файлы:
- [`handler.c`](https://github.com/dangerUser45/OS_tasks/blob/main/11th_seminar/src/handler.c)  
- [`signal_cat.c`](https://github.com/dangerUser45/OS_tasks/blob/main/11th_seminar/src/signal_cat.c)

---

# 12-й семинар  
📂 **[12th_seminar](https://github.com/dangerUser45/OS_tasks/tree/main/12th_seminar/src)**  

Тема: **real-time сигналы (sigqueue + SA_SIGINFO)**

### Файл:
- [`chat.c`](https://github.com/dangerUser45/OS_tasks/blob/main/12th_seminar/src/chat.c)

---

# Библиотека safe_lib  
📂 **[lib](https://github.com/dangerUser45/OS_tasks/tree/main/lib)**  

- [`safe_lib.c`](https://github.com/dangerUser45/OS_tasks/blob/main/lib/src/safe_lib.c)  
- [`safe_lib.h`](https://github.com/dangerUser45/OS_tasks/blob/main/lib/include/safe_lib.h)  
- [`color.h`](https://github.com/dangerUser45/OS_tasks/blob/main/lib/include/color.h)

Используется для безопасных обёрток и цветного вывода.
