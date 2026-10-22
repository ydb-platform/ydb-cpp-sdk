use strict;
use warnings;
use TAP::Parser;
use JSON::PP;
use File::Basename qw(basename);
my $path = shift;
my $parser = TAP::Parser->new({exec => [$^X, '-I', 't', $path], merge => 1});
my (@problems, @rows);
while (my $result = $parser->next) {
    print $result->raw, "\n";
    if ($result->is_test) {
        push @rows, {number => $result->number,
                     status => $result->has_skip || $result->has_todo ? 'skipped' : $result->is_ok ? 'passed' : 'failed',
                     description => $result->description};
        push @problems, {message => $result->raw} if !$result->is_ok || $result->has_skip || $result->has_todo;
    }
    push @problems, {message => $result->raw} if $result->is_bailout;
}
push @problems, {message => $parser->skip_all} if $parser->skip_all;
my $status = $parser->has_problems ? 'failed' : @problems ? 'skipped' : 'passed';
# Test::More exits with its failed assertion count, capped at 254.
my $failed = $parser->failed;
my $failure_exit = $failed > 254 ? 254 : $failed;
$status = 'broken' if ($parser->exit > 1 && $parser->exit != $failure_exit)
                    || ($parser->wait & 127) || $parser->parse_errors;
open my $fh, '>', "$ENV{ODBC_NATIVE_RESULTS_DIR}/" . basename($path) . '.json' or die $!;
print $fh encode_json({status => $status, rows => \@rows, problems => \@problems,
                      message => join("\n", map { $_->{message} } @problems, map { {message=>$_} } $parser->parse_errors)});
