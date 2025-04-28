#!/bin/sh

echo $(date) >> date-out.txt
echo '#!/bin/sh' > ~/date-script.sh
echo 'echo $(date) >> date-out.txt' >> ~/date-script.sh
chmod +x ~/date-script.sh