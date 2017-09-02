#!/bin/sh

# Create test files and directories

test -e test && {
    echo "Remove test and try again"
    exit 1
}

mkdir outdir && cd outdir

echo 'It works!' > normal.txt
echo 'Με δουλέβει;' > 'κοινό.txt'
ln -s normal.txt ln-normal.txt
ln -s normal.txt ln-normal
mkdir normal-dir
ln -s normal-dir ln-normal-dir
ln -s nowhere ln-nowhere
mkfifo mk-fifo
touch no-access && chmod 000 no-access
mkdir no-access-dir && chmod 000 no-access-dir
ln -s ../normal.txt normal-dir/ln-normal.txt
ln -s ../normal.txt normal-dir/ln-normal
echo 'int main(void) { *((char *)0) = 0; }' > ill.c
make ill > /dev/null
echo 'test/ill' > ill.sh
mkdir empty-dir
mkdir cage
echo 'chmod 000 test/cage' > cage/lock.sh
echo 'chmod 755 test/cage' > cage-unlock.sh
mkdir cage/lion
echo 'chmod 000 test/cage' > cage/lion/lock.sh
mkdir unicode
touch 'unicode/Malgudi Days - मालगुडी डेज - E05. Swami and Friends - स्वामी और उसके दोस्त (Part 1)'
touch 'unicode/Malgudi Days - मालगुडी डेज - E05. Swami and Friends - स्वामी और उसके दोस्त (Part 2)'
touch 'unicode/Malgudi Days - मालगुडी डेज - E05. Swami and Friends - स्वामी और उसके दोस्त (Part 3)'
chmod +x 'unicode/Malgudi Days - मालगुडी डेज - E05. Swami and Friends - स्वामी और उसके दोस्त (Part 2)'
touch 'unicode/Führer'
touch 'unicode/Eso eso aamar ghare eso ♫ এসো এসো আমার ঘরে এসো ♫ Swagatalakshmi Dasgupta'
