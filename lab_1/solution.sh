rm -r 9091
mkdir 9091 && cd 9091
mkdir Vinogradov && cd Vinogradov
date > Yura.txt
date --date "next Mon" > filedate.txt
cat Yura.txt filedate.txt > result.txt
cat result.txt