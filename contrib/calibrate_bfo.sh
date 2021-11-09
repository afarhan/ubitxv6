#!/bin/sh

bfo=11048000

max=11060000

echo -n Freq:
ubitx_client -c get_frequency

while [ ${bfo} -lt ${max} ]
do
bfo=$((bfo+50))
echo ${bfo}
ubitx_client -c set_bfo -a ${bfo}
sleep 4
done
