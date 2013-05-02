set ns [new Simulator]

# Set routing protocol
#$ns rtproto DV

# Open nam tracefile
set nf [open prob1.nam w]

# Open tracefile
#set nt [open trace-ST.tr w]
set nt [open trace-DV.tr w]

$ns namtrace-all $nf
$ns trace-all $nt

# create 7 nodes
puts "create 7 nodes now....."

set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]
set n4 [$ns node]
set n5 [$ns node]
set n6 [$ns node]

#for {set i 0} {$i < 7} {incr i} {
#set n($i) [$ns node]
#}
#for {set i 0} {$i < 7} {incr i} {
#$ns duplex-link $n($i) $n([expr ($i+1)%7]) 1Mb 10ms DropTail
#}

puts "create connections now....."

# Create connection
$ns duplex-link $n0 $n1 1.5Mb 10ms DropTail
$ns duplex-link $n1 $n2 1.5Mb 10ms DropTail
$ns duplex-link $n2 $n3 1.5Mb 10ms DropTail
$ns duplex-link $n3 $n4 1.5Mb 10ms DropTail
$ns duplex-link $n4 $n5 1.5Mb 10ms DropTail
$ns duplex-link $n5 $n6 1.5Mb 10ms DropTail
$ns duplex-link $n6 $n0 1.5Mb 10ms DropTail

puts "Create agents and attach to appropriate nodes..."
#Create a UDP agent and attach it to node n0
set udp0 [new Agent/UDP]
$ns attach-agent $n0 $udp0

# Create a CBR traffic source and attach it to udp0
set cbr0 [new Application/Traffic/CBR]
$cbr0 set packetSize_ 500
$cbr0 set interval_ 0.005
$cbr0 attach-agent $udp0

#Create a Null agent (a traffic sink) and attach it to node n1
set null0 [new Agent/Null]
$ns attach-agent $n3 $null0
#Connect the traffic source with the traffic sink
$ns connect $udp0 $null0  

#Schedule events for the CBR agent
$ns at 0.0 "$cbr0 start"
$ns at 4.5 "$cbr0 stop"
#Call the finish procedure after 5 seconds of simulation time
$ns at 5.0 "finish"

puts "schedule transmitting packets..."

puts "link down node2 and node3............"
$ns rtmodel-at 1.0 down $n1 $n2
puts "link up node2 and node3............"
$ns rtmodel-at 2.0 up $n1 $n2


#Define a 'finish' procedure
proc finish {} {
global ns nf nt
$ns flush-trace
close $nf
close $nt
exec nam -a prob1.nam &
exit 0
}

$ns at 5.00 "finish"
$ns run
