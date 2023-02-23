# wireless 802.11 (static)
# wireless 802.15.4 (mobile)

# simulator
set ns [new Simulator]

# define options
set val(chan)         Channel/WirelessChannel   ;# channel type
set val(prop)         Propagation/TwoRayGround  ;# radio-propagation model
set val(ant)          Antenna/OmniAntenna       ;# Antenna type
set val(ll)           LL                        ;# Link layer type
set val(ifq)          Queue/DropTail/PriQueue   ;# Interface queue type
set val(ifqlen)       50                        ;# max packet in ifq
set val(netif)        Phy/WirelessPhy           ;# network interface type
set val(mac)          Mac/802_11                ;# MAC type
set val(rp)           AODV                      ;# ad-hoc routing protocol 

set val(size)         [lindex $argv 0]           ;# grid size
set val(nn)           [lindex $argv 1]           ;# number of mobilenodes
set val(nf)           [lindex $argv 2]           ;# number of flows

# trace file
set trace_file [open "trace.tr" w]
$ns trace-all $trace_file

# nam file 
set nam_file [open "animation.nam" w]
$ns namtrace-all-wireless $nam_file $val(size) $val(size)

# topology: to keep track of node movements
set topo [new Topography]
$topo load_flatgrid $val(size) $val(size)

# general operation director for mobilenodes
create-god $val(nn)

# node configs
$ns node-config -adhocRouting $val(rp) \
                -llType $val(ll) \
                -macType $val(mac) \
                -ifqType $val(ifq) \
                -ifqLen $val(ifqlen) \
                -antType $val(ant) \
                -propType $val(prop) \
                -phyType $val(netif) \
                -topoInstance $topo \
                -channel [new $val(chan)] \
                -agentTrace ON \
                -routerTrace ON \
                -macTrace ON \
                -movementTrace ON
                -energyModel EnergyModel \
                -rxPower 0.3 \
                -txPower 0.6 \
                -initialEnergy 5 \
                -idlePower 0.2 \
                -sleepPower 0.1 \
                # -radioSwitchingPower 0.1 \
                -energyTrace ON

# create nodes
set row 0
set col 0
for {set i 0} {$i < $val(nn)} {incr i} {
    set node($i) [$ns node]

    # disabling random motion
    $node($i) random-motion 0

    # positioning nodes along x & y axes
    $node($i) set X_ [expr (5000 * $col) / $val(nn)]
    $node($i) set Y_ [expr ($row) / $val(nn)]
    $node($i) set Z_ 0

    # configuring node
    $ns initial_node_pos $node($i) 20

    # incrementing row & col
    incr col
    if {$i % 10 == 9} {
        set row [expr $row + 5000]
        set col 0
    }
}

# producing node movements with uniform random speed
for {set i 0} {$i < $val(nn)} {incr i} {
    $ns at [expr int(20 * rand()) + 10] "$node($i) setdest [expr int(10000 * rand()) % $val(size) + 0.5] [expr int(10000 * rand()) % $val(size) + 0.5] [expr int(100 * rand()) % 5 + 1]"
}


# generating traffic/flow

# picking random source node
set src [expr int(10000 * rand()) % $val(nn)]

# picking random destination node
for {set i 0} {$i < $val(nf)} {incr i} {
    while {$src == $src} {
        set dest [expr int(10000 * rand()) % $val(nn)]
        if {$src != $dest} {
            break
        }
    }


    # create agent
    set tcp [new Agent/TCP]               ;# TCP Tahoe
    set tcp_sink [new Agent/TCPSink]
    # attach to nodes
    $ns attach-agent $node($src) $tcp
    $ns attach-agent $node($dest) $tcp_sink
    # connect agents
    $ns connect $tcp $tcp_sink
    $tcp set fid_ $i

    # Traffic generator
    set telnet [new Application/Telnet]
    # attach to agent
    $telnet attach-agent $tcp
    
    # start traffic generation
    # start between 1 to 9 seconds
    $ns at [expr int(9 * rand()) + 1] "$telnet start"
}

# ending simulation
# stopping nodes
for {set i 0} {$i < $val(nn)} {incr i} {
    $ns at 50.0 "$node($i) reset"
}

# call final function
proc finish {} {
    global ns trace_file nam_file
    $ns flush-trace
    close $trace_file
    close $nam_file
}

proc halt_simulation {} {
    global ns
    puts "Simulation ending"
    $ns halt
}

$ns at 50.0001 "finish"
$ns at 50.0002 "halt_simulation"

# Run simulation
puts "Simulation starting"
$ns run