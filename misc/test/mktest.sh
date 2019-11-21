#!/bin/sh

# Create test files and directories

test -e outdir && {
    echo "Remove 'outdir' and try again"
    exit 1
}

mkdir -p outdir && cd outdir || exit 1

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
mkdir -p korean
touch 'korean/[ENG sub] PRODUCE48 울림ㅣ김채원ㅣ행복 나눠주는 천사소녀 @자기소개_1분 PR 180615 EP.0-Cgnmr6Fd82'
touch 'korean/[ENG sub] PRODUCE48 [48스페셜] 윙크요정, 내꺼야!ㅣ김채원(울림) 180615 EP.0-K7ulTiuJZK8.mp4'
touch 'korean/[FULL ENG SUB] 181008 SALEWA x IZ_ONE Long Padding Photoshoot Behind Film-[오늘의 시구] 아이즈원 (IZONE) 장원영&미야와키 사쿠라! 시구 시타! (10.06)-VmDl5eBJ3x0.mkv'
touch 'korean/IZ_ONE (아이즈원) - 1st Mini Album [COLOR_IZ] Highlight Medley-w9V2xFrYIgk.web'
touch 'korean/IZ_ONE (아이즈원) - 1st Mini Album [COLOR_IZ] MV TEASER 1-uhnJLBNBNto.mkv'
touch 'korean/IZ_ONE CHU [1회] ′순도 100%′ 우리즈원 숙소 생활 ★최초 공개★ 181025 EP.1-pcutrQN1Sbg.mkv'
touch 'korean/IZ_ONE CHU [1회_예고] 아이즈원 데뷔 준비 과정 ★독점 공개★ 아이즈원 츄 이번주 (목) 밤 11시 첫방송 181025'
touch 'korean/IZ_ONE CHU [1회] 도치기현 1호 이모 팬과의 만남! 181025 EP.1-5kYoReT5x44.mp4'
touch 'korean/IZ_ONE CHU [1회] ′12명 소녀들의 새로운 시작′ 앞으로 아이즈원 잘 부탁해♥ 181025 EP.1-RVNvgbdLQLQ'
touch 'korean/IZ_ONE CHU [1회] ′앗..그것만은!′ 자비없는 합숙생활 폭로전 181025 EP.1-AmP5KzpoI38.mkv'
touch 'korean/IZ_ONE CHU [1회] 휴게소 간식 내기 노래 맞히기 게임 181025 EP.1-LyNDKflpWYE.mp4'
touch 'korean/IZ_ONE CHU [1회] 2018 아이즈원 걸크러시능력시험 (feat. 치타쌤) 181025 EP.1-9qHWpbo0eB8.mp4'
touch 'korean/IZ_ONE CHU [1회] ′돼지요′ 아니죠, ′되지요′ 맞습니다! (feat. 꾸라먹방) 181025EP.1-WDLFqMWiKn'
touch 'korean/IZ_ONE CHU [1회] ′두근두근′ 첫 MT를 앞둔 비글력 만렙의 아이즈원 181025 EP.1'
mkdir -p unicode
touch 'unicode/Malgudi Days - मालगुडी डेज - E05. Swami and Friends - स्वामी और उसके दोस्त (Part 1)'
touch 'unicode/Malgudi Days - मालगुडी डेज - E05. Swami and Friends - स्वामी और उसके दोस्त (Part 2)'
touch 'unicode/Malgudi Days - मालगुडी डेज - E05. Swami and Friends - स्वामी और उसके दोस्त (Part 3)'
chmod +x 'unicode/Malgudi Days - मालगुडी डेज - E05. Swami and Friends - स्वामी और उसके दोस्त (Part 2)'
touch 'unicode/Führer'
touch 'unicode/Eso eso aamar ghare eso ♫ এসো এসো আমার ঘরে এসো ♫ Swagatalakshmi Dasgupta'
touch 'max_chars_filename_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'
