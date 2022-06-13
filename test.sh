#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: test.sh <out file> <in file>"
    exit 1
fi

outF=$1
inF=$2

securities_in=$(cat $inF | awk -F, '{print $2}'|sort | uniq)
securities_out=$(cat $outF | awk -F, '{print $1}')
diff <(echo "$securities_in") <(echo "$securities_out")
if [ $? -ne 0 ]; then
    echo "securities are not in a sorted order => FAIL"
    exit 1
fi

while read -r line; do
   read -r ticker gap vol wap price <<<"$(echo $line | tr ',' ' ')"
   ticker_data="$(grep $ticker $inF)"
   agap=$(echo "$ticker_data"|awk -F, 'NR>1{print $1-p} {p=$1}'|sort -n| tail -n 1)
   if [ $agap -ne $gap ]; then
       echo "max time gap mismatch for security $ticker: $agap != $gap => FAIL"
       exit 1
   fi
   avol=$(echo "$ticker_data"|awk -F, '{sum+=$3;}END{print sum;}')
   if [ $avol -ne $vol ]; then
       echo "total volume mismatch for security $ticker: $avol != $vol => FAIL"
       exit 1
   fi
   awap=$(echo "$ticker_data"|awk -F, -v v=$avol '{sum+=$3*$4;}END{printf("%d",sum/v);}')
   if [ $awap -ne $wap ]; then
       echo "weighted average price mismatch for security $ticker: $awap != $wap => FAIL"
       exit 1
   fi
   aprice=$(echo "$ticker_data"|awk -F, 'BEGIN{max=0}{if ($4>max) max=$4}END{print max}')
   aprice=${aprice::${#aprice}-1}
   #${#aprice}
   #echo "$ticker $gap $vol $wap $price $aprice"
   #echo "$ticker $gap $expected_gap $expected_price $price"
   if [ $aprice -ne $price ]; then
      echo "max price mismatch for security $ticker: $aprice != $price => FAIL"
      exit 1
   fi
done < "$outF"
echo "PASS"
