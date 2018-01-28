#!/usr/bin/perl -w

# Runs the regression test suite.

use strict;
use Carp;
use IO::Handle;
use IPC::Open2;

use constant ecss =>
    'LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./bin/ecss';

use constant st_start  => 'st_start';
use constant st_begin  => 'st_begin';
use constant st_input  => 'st_input';
use constant st_output => 'st_output';

my @filenames;
my $tests_run = 0;
my $tests_failed = 0;

#
# Run the "*.t" tests. These files may contain multiple tests, delimited by
# "-begin -i [input] -o [expected output] -end".
#

my @tests = ();
my $state = st_start;
my $test;

@filenames = map { chomp; $_ } `find tests -name \*.t`;
foreach my $filename (@filenames) {
    open FH, $filename or die "couldn't open $filename for reading: $!";
    my @lines = map { chomp; $_ } <FH>;
    foreach my $i (0..$#lines) {
        my $s = $lines[$i];
        if ($s eq '-begin') {
            assert_state(st_start, $filename, $i);
            $state = st_begin;
            $test = {where => "$filename on line " . ($i+1)};
        }
        elsif ($s eq '-i') {
            assert_state(st_begin, $filename, $i);
            $state = st_input;
            $test->{input} = "";
        }
        elsif ($s eq '-o') {
            assert_state(st_input, $filename, $i);
            $state = st_output;
            $test->{output} = "";
        }
        elsif ($s eq '-end') {
            assert_state(st_output, $filename, $i);
            $state = st_start;
            push @tests, $test if defined $test;
            $test = undef;
        }
        else {
            if ($state eq st_input) {
                $test->{input} .= "$s\n";
            }
            elsif ($state eq st_output) {
                $test->{output} .= "$s\n";
            }
        }
    }
}

my @failures;
foreach my $test (@tests) {
    run_test($test);
}
if (@failures) {
    my $sep = ('-' x 20) . "\n";
    print $sep . join($sep, @failures) . $sep;
}

#
# Run the tests organized as in/out file pairs.
#

@filenames = map { chomp; $_ } `find tests -name \\*.in`;
for my $in (@filenames) {
   my $out;
   ($out = $in) =~ s/\.in$/.out/;
   if (system(ecss . " -i$in | diff -u $out - 2>&1") != 0) {
       print "FAILED: $in => $out\n";
       $tests_failed++;
   }
   $tests_run++;
}

printf("%d of %d tests passed.\n", $tests_run-$tests_failed, $tests_run);
exit($tests_failed ? 2 : 0);

#-------------#
# subroutines #
#-------------#

sub assert_state {
    my ($expected, $filename, $i) = @_;
    confess("syntax error in $filename:$i") unless $state eq $expected;
}

sub run_test {
    my $test = shift;
    my ($reader, $writer);
    ($reader, $writer) = (IO::Handle->new, IO::Handle->new);
    open2($reader, $writer, ecss);
    $writer->print($test->{input});
    $writer->close();
    my $actual = join("", $reader->getlines());
    if ($test->{output} ne $actual) {
        push @failures, sprintf("FAILED\n%s\nEXPECTED\n%sACTUAL\n%s",
                                $test->{where}, $test->{output}, $actual);
        $tests_failed++;
    }
    $tests_run++;
}

