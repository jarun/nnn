#!/bin/sh

# Create test files and directories

test -e outdir && {
    echo "Remove 'outdir' and try again"
    exit 1
}

mkdir -p outdir && cd outdir

echo 'It works!' > normal.txt
echo 'Με δουλέβει;' > 'κοινό.txt'
ln -sf normal.txt ln-normal.txt
ln -sf normal.txt ln-normal
mkdir -p normal-dir
ln -sf normal-dir ln-normal-dir
ln -sf nowhere ln-nowhere
mkfifo mk-fifo
touch no-access && chmod 000 no-access
mkdir -p no-access-dir && chmod 000 no-access-dir
ln -sf ../normal.txt normal-dir/ln-normal.txt
ln -sf ../normal.txt normal-dir/ln-normal
echo 'int main(void) { *((char *)0) = 0; }' > ill.c
make ill > /dev/null
echo 'test/ill' > ill.sh
mkdir -p empty-dir
mkdir -p cage
echo 'chmod 000 test/cage' > cage/lock.sh
echo 'chmod 755 test/cage' > cage-unlock.sh
mkdir -p cage/lion
echo 'chmod 000 test/cage' > cage/lion/lock.sh
mkdir -p unicode
touch 'unicode/Malgudi Days - मालगुडी डेज - E05. Swami and Friends - स्वामी और उसके दोस्त (Part 1)'
touch 'unicode/Malgudi Days - मालगुडी डेज - E05. Swami and Friends - स्वामी और उसके दोस्त (Part 2)'
touch 'unicode/Malgudi Days - मालगुडी डेज - E05. Swami and Friends - स्वामी और उसके दोस्त (Part 3)'
chmod +x 'unicode/Malgudi Days - मालगुडी डेज - E05. Swami and Friends - स्वामी और उसके दोस्त (Part 2)'
touch 'unicode/Führer'
touch 'unicode/Eso eso aamar ghare eso ♫ এসো এসো আমার ঘরে এসো ♫ Swagatalakshmi Dasgupta'
touch 'max_chars_filename_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'
