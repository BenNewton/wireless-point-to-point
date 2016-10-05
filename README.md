This is an ns-3 model that can simulate "perfect" directional links which act 
like point to point links, but are wireless, so that multiple devices are 
attached to a single channel, and connections between devices are explicitly 
specified.  

Based off of ns-3.26 

To use, clone the contents into the ns-3.26/src directory

Like this:
cd ns-3.26/src
git clone https://github.com/BenNewton/wireless-point-to-point.git
cd ..
./waf configure --enable-examples
./waf --run=wireless-point-to-point-example


