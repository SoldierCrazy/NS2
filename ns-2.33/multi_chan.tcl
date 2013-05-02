# VARIABLE PART

set val(chan)           Channel/WirelessChannel    ;# channel type
set val(prop)           Propagation/TwoRayGround   ;# radio-propagation model
set val(netif)          Phy/WirelessPhy            ;# network interface type
set val(mac)            Mac/802_11                 ;# MAC type
set val(ifq)            Queue/DropTail/PriQueue    ;# interface queue type
set val(ll)             LL                         ;# link layer type
set val(ant)            Antenna/OmniAntenna        ;# antenna model
set val(ifqlen)         50                         ;# max packet in ifq
set val(nn)             6                          ;# number of mobilenodes
set val(rp)            AODV                      ;# routing protocol
set val(x)      2000
set val(y)      2000
set val(ni)		6
set simtime  500
set opt(sc)    "close.ns_movements" 
set opt(sampleTime) 0.5
set filename       twoflows-2channel


# TRACE PART

set ns_     [new Simulator]
#
$ns_ color 1 Blue
$ns_ color 2 Red

set tracefd     [open $filename.tr w]
#$ns_ use-newtrace
$ns_ trace-all $tracefd

#set tracefile [open $filename.tr w]
#$ns_ trace-all $tracefile

set namtrace [open $filename.nam w]
$ns_ namtrace-all-wireless $namtrace $val(x) $val(y)

set topo [new Topography]
$topo load_flatgrid $val(x) $val(y)

set chan_(0) [new $val(chan)]
set chan_(1) [new $val(chan)]
set chan_(2) [new $val(chan)]
set chan_(3) [new $val(chan)]
set chan_(4) [new $val(chan)]
set chan_(5) [new $val(chan)]


create-god [expr $val(nn)*$val(ni)]

# NODE CONFIG PART


$ns_ node-config -adhocRouting $val(rp) \
		-llType $val(ll) \
		-macType $val(mac) \
		-ifqType $val(ifq) \
		-ifqLen $val(ifqlen) \
		-antType $val(ant) \
		-propType $val(prop) \
		-phyType $val(netif) \
		-channel chan_(0) \
		-topoInstance $topo \
		-agentTrace ON \
		-routerTrace ON \
		-macTrace ON \
		-movementTrace OFF \
		-ifNum $val(ni)


#$ns_ change-numifs $val(ni)

#for {set i 0} {$i < $val(ni) } {incr i} {
#$ns_ add-channel $i $chan_($i)
#}

#for {set i 0} {$i < $val(nn) } {incr i} {
#set node_($i) [$ns_ node]
#$node_(0) random-motion 0
#}

$ns_ change-numifs 3
$ns_ add-channel 0 $chan_(0)
$ns_ add-channel 1 $chan_(1)
$ns_ add-channel 2 $chan_(2)
set node_(0) [$ns_ node]
$node_(0) random-motion 0

$ns_ change-numifs 2
$ns_ add-channel 0 $chan_(3)
$ns_ add-channel 1 $chan_(1)
set node_(1) [$ns_ node]
$node_(1) random-motion 0

$ns_ change-numifs 2
$ns_ add-channel 0 $chan_(0)
$ns_ add-channel 1 $chan_(4)
set node_(2) [$ns_ node]
$node_(2) random-motion 0

$ns_ change-numifs 1
$ns_ add-channel 0 $chan_(3)
#$ns_ add-channel 1 $chan_(0)
set node_(3) [$ns_ node]
$node_(3) random-motion 0

$ns_ change-numifs 1
$ns_ add-channel 0 $chan_(4)
#$ns_ add-channel 1 $chan_(1)
#$ns_ add-channel 3 $chan_(3)
set node_(4) [$ns_ node]
$node_(4) random-motion 0

$ns_ change-numifs 2
$ns_ add-channel 0 $chan_(5)
$ns_ add-channel 1 $chan_(2)
set node_(5) [$ns_ node]
$node_(5) random-motion 0

$ns_ change-numifs 2
$ns_ add-channel 0 $chan_(5)
$ns_ add-channel 1 $chan_(2)
set node_(6) [$ns_ node]
$node_(6) random-motion 0

$ns_ change-numifs 2
$ns_ add-channel 0 $chan_(5)
$ns_ add-channel 1 $chan_(2)
set node_(7) [$ns_ node]
$node_(7) random-motion 0

$node_(0) set X_ 716.3675547375121
$node_(0) set Y_ 1488.038297377213
$ns_ at 0.0 "$node_(0) setdest 705.8078756806557 1489.2759330716674 9.5084318299731072"

$node_(1) set X_ 635.0024287310823
$node_(1) set Y_ 1450.5482822475094
$ns_ at 0.0 "$node_(1) setdest 629.5533867954466 1461.4908515775323 12.4070384992638115"

$node_(2) set X_ 643.1581099747762
$node_(2) set Y_ 1534.7861859264958
$ns_ at 0.0 "$node_(2) setdest 647.7331506039237 1512.9285351521874 13.0071730248338697"

$node_(3) set X_ 572.6556913997405			;#node helper
$node_(3) set Y_ 1470.4439495143297
$ns_ at 0.0 "$node_(3) setdest 574.3890027551888 1482.604082850264 9.0059103700738177"

$node_(4) set X_ 576.6556913997405			;#node 8
$node_(4) set Y_ 1512.4439495143297
$ns_ at 0.0 "$node_(4) setdest 570.3890027551888 1518.604082850264 9.0059103700738177"

$node_(5) set X_ 653.6556913997405			;#node helper
$node_(5) set Y_ 1493.4439495143297
$ns_ at 0.0 "$node_(5) setdest 633.3890027551888 1490.604082850264 13.0059103700738177"

#$node_(6) set X_ 601.6556913997405			;#node helper
#$node_(6) set Y_ 1537.4439495143297
#$ns_ at 0.0 "$node_(6) setdest 586.3890027551888 1541.604082850264 2.0059103700738177"

#$node_(7) set X_ 607.6556913997405			;#node helper
#$node_(7) set Y_ 1491.4439495143297
#$ns_ at 0.0 "$node_(7) setdest 582.3890027551888 1499.604082850264 2.0059103700738177"

#$ns_ change−numifs $val(ni)

#for {set i 0} {$i < $val(ni) } {incr i} {
#$ns_ add-channel $i $chan_($i)
#}

#for {set i 0} {$i < $val(nn) } {incr i} {
#set node_($i) [$ns_ node]
#$node_(0) random-motion 0
#}

# GRAPH PART

#for {set i 0} {$i < $val(nn) } {incr i} {
#    $ns_ initial_node_pos $node_($i) 10
#}

#$node_(0) add-route-tag 1 999
#$node_(2) add-route-tag 3 8

# Traffic
proc attach-cbr-traffic { node sink size interval con } {
	global ns_
	set source [new Agent/UDP]
	$source set class_ $con
	$ns_ attach-agent $node $source
	set traffic [new Application/Traffic/CBR]
    $traffic set class_ $con
	$traffic set packetSize_ $size
	$traffic set interval_ $interval
	$traffic attach-agent $source
	$ns_ connect $source $sink
#set bwt [ open bwt.$con.tr w ] ; #$ opt (bwtDelimiter ) .
#$ns_ at 0.5 "record $sink $bwt"
#puts "added TFRC connection number $con (cbr($con)) "
	return $traffic
}

proc record { sink bwt } {
global opt val
set ns_ [Simulator instance]
set time $opt(sampleTime)
#How many bytes have been received by the traffic sink ?
set bw [$sink set bytes_]
# Get the current time
set now [$ns_ now]
#Open a file to write bandwidth − trace
# Calculate the bandwidth ( in MBit / s ) and write it to the file
puts $bwt "$now [expr $bw/$time*8/1000000]"
# Reset the bytes values on the traffic sinks
$sink set bytes_ 0
# Re − schedule the procedure
$ns_ at [expr $now+$time] "record $sink $bwt"
}


$ns_ change-numifs 3
set null0 [new Agent/Null]
$ns_ attach-agent $node_(0) $null0
$ns_ change-numifs 1
set cbr0 [attach-cbr-traffic $node_(3) $null0 500 0.008 0]
$ns_ at 0.001 "$cbr0 start"
$ns_ at 4.9 "$cbr0 stop"

set null1 [new Agent/Null]
$ns_ attach-agent $node_(0) $null1
set cbr1 [attach-cbr-traffic $node_(4) $null1 500 0.008 1]
$ns_ at 0.001 "$cbr1 start"
$ns_ at 4.9 "$cbr1 stop"

set null2 [new Agent/Null]
$ns_ attach-agent $node_(0) $null2
set cbr2 [attach-cbr-traffic $node_(5) $null2 500 0.008 2]
$ns_ at 0.001 "$cbr2 start"
$ns_ at 4.9 "$cbr2 stop"

for {set i 0} {$i < $val(nn) } {incr i} {
    $ns_ at 15.0 "$node_($i) reset";
}

proc finish {} {
	global ns_ tracefd namtrace
	$ns_ flush-trace
	close $tracefd
	close $namtrace
  	exit 0
}
puts "Starting Simulation..."

$ns_ at [expr $simtime+0.01] "finish"
$ns_ run
