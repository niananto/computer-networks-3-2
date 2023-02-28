# wireless 802.15.4 (mobile)
if {$argc != 5} {
    puts "Usage: ns $argv0 <#nodes> <#flows> <#packets> <speed of nodes> <agent type>"
    exit 1
}

# simulator
set ns [new Simulator]

# define options
set val(chan)         Channel/WirelessChannel
set val(prop)         Propagation/TwoRayGround
set val(ant)          Antenna/OmniAntenna
set val(ll)           LL
set val(ifq)          Queue/DropTail/PriQueue
set val(ifqlen)       50
set val(netif)        Phy/WirelessPhy/802_15_4
set val(mac)          Mac/802_15_4
set val(rp)           DSDV

set val(energymodel)  EnergyModel
set val(initenergy)   12

set val(size)         500
set val(start)        0
set val(stop)         100

set val(nn)           [lindex $argv 0]
set val(nf)           [lindex $argv 1]
set val(npkt)         [lindex $argv 2]
set val(spd)          [lindex $argv 3]
set val(agent)        [lindex $argv 4]


# trace file
set trace_file [open "trace.tr" w]
$ns trace-all $trace_file

set second_trace_file [open "trace2.tr" w]

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
                -macTrace OFF \
                -movementTrace OFF \
                -energyModel $val(energymodel) \
                -initialEnergy $val(initenergy) \
                -rxPower 1.0 \
                -txPower 1.0 \
                -idlePower 1.0 \
                -sleepPower 0.001 \
                -rate 250000 \
                # -IncomingErrProc [new ErrorModel/Uniform 2 pkt] \

expr srand(93)
# Generate nodes at random positions
set points []
for {set i 0} {$i < $val(nn)} {incr i} {
    set x [expr rand() * $val(size)]
    set y [expr rand() * $val(size)]

    while {[lsearch -all -inline $points [list $x $y]] != {}} {
        set x [expr rand() * $val(size)]
        set y [expr rand() * $val(size)]
    }

    set point [list $x $y]
    lappend points $point

    set node_($i) [$ns node]
    $node_($i) random-motion 0
    
    # Set initial node position
    $node_($i) set X_ $x
    $node_($i) set Y_ $y
    $node_($i) set Z_ 0.0
    $ns initial_node_pos $node_($i) 20 

    # set transmission power
    $node_($i) set txpower_ 10

    # set transmission range
    # $node_($i) set txrange_ 100

    # set propagation constant
    $node_($i) set propagationConstant 0.3
}

# producing node movements with uniform random speed
for {set i 0} {$i < $val(nn)} {incr i} {
    $ns at $val(start) "$node_($i) setdest [expr int(10000 * rand()) % $val(size) + 0.5] \
                                        [expr int(10000 * rand()) % $val(size) + 0.5] \
                                        $val(spd)"
}


# generating traffic/flow
# picking random source node
# picking random destination node
for {set i 0} {$i < $val(nf)} {incr i} {
    set src_($i) [expr int(rand() * $val(nn))]
    set dest_($i) [expr int(rand() * $val(nn))]
    while {$dest_($i) == $src_($i)} {
        set dest_($i) [expr int(rand() * $val(nn))]
    }

    # create agent
    set tcp_($i) [new Agent/TCP/Reno]
    set sink_($i) [new Agent/TCPSink]
    
    if {$val(agent) == "areno"} {
      set tcp_($i) [new Agent/TCP/AReno]
      $tcp_($i) set window_ 10
      set sink_($i) [new Agent/TCPSink/Sack1]
    }
    if {$val(agent) == "westwood"} {
      set tcp_($i) [new Agent/TCP/Westwood]
      $tcp_($i) set window_ 10
      set sink_($i) [new Agent/TCPSink/Sack1]

      # set tcp_($i) [new Agent/TCP/Linux]
      # $ns at 0 "$tcp_($i) select_ca westwood"
      # set sink_($i) [new Agent/TCPSink/Sack1]
      # $sink_($i) set ts_echo_rfc1323_ true
    }
    if {$val(agent) == "newreno"} {
      set tcp_($i) [new Agent/TCP/Newreno]
      set sink_($i) [new Agent/TCPSink]
    }
    if {$val(agent) == "westwoodnr"} {
      set tcp_($i) [new Agent/TCP/WestwoodNR]
      $tcp_($i) set window_ 10
      set sink_($i) [new Agent/TCPSink]
    }
    
    # attach to nodes
    $ns attach-agent $node_($src_($i)) $tcp_($i) 
    $ns attach-agent $node_($dest_($i)) $sink_($i)
    
    $tcp_($i) set maxseq_ $val(npkt)
    # $tcp_($i) set maxcwnd_ 100

    # connect agents
    $ns connect $tcp_($i) $sink_($i)
    $tcp_($i) set fid_ $i

    $tcp_($i) attach $second_trace_file
    $tcp_($i) trace cwnd_
    $tcp_($i) trace ssthresh_
    $tcp_($i) trace rtt_
    $tcp_($i) trace ack_
    $tcp_($i) trace maxseq_

    # Traffic generator
    set ftp_($i) [new Application/FTP]
    # attach to agent
    $ftp_($i) attach-agent $tcp_($i)    
    # start traffic generation
    $ns at $val(start) "$ftp_($i) start"
}

# ending simulation
# stopping nodes
for {set i 0} {$i < $val(nn)} {incr i} {
    $ns at $val(stop) "$node_($i) reset"
}

# call final function
proc finish {} {
    global ns trace_file nam_file second_trace_file
    $ns flush-trace
    close $trace_file
    close $second_trace_file
    close $nam_file
    puts "Simulation ending"
    $ns halt
}

$ns at $val(stop) "finish"

# Run simulation
puts "Simulation starting"
$ns run