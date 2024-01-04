#! /bin/bash

# Only tested on Ubuntu 20.04 Linux so far

sudo apt-get update
sudo apt-get install python3-venv
cd $HOME
python3 -m venv venv-dpugen
source $HOME/venv-dpugen/bin/activate

cd $HOME
git clone https://github.com/mgheorghe/dpugen
cd dpugen
pip3 install -r requirements.txt

F="$HOME/dpugen-setup.bash"
cp /dev/null ${F}
echo "source \$HOME/venv-dpugen/bin/activate" > ${F}
echo "export PYTHONPATH=\"\$HOME/dpugen:\$HOME/venv-dpugen/lib/python3.8/site-packages\"" > ${F}
