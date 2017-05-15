# ns3learning
netconvert --osm-files map.osm.xml -o map.net.xml

polyconvert --net-file map.net.xml --osm-files map.osm --type-file typemap.xml -o map.poly.xml

python /SUMO_HOME/tools/randomTrips.py -n map.net.xml -e 100 -l

python /SUMO_HOME/tools/randomTrips.py -n map.net.xml -r
                                          map.rou.xml e 100 -l
sumo-gui map.sumo.cfg

python /SUMO_HOME/traceExporter.py --fcd-input map.sumo.xml 
                                   --ns2config-output map.tcl
                                   --ns2mobility-output mobility.tcl
                                   --ns2activity-output activity.tcl
