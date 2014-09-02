#!/usr/bin/perl -w
use strict;

use DBI;
use Time::Piece;
use Data::Dumper;

my %db = (
	host	=> 'green-swamp.ru',
	port	=> 3306,
	user	=> 'pacs1',
	pass	=> 'JfzpjuFRva',
	name	=> 'db_pacs1'
);
my $dbh = DBI->connect ("DBI:mysql:database=$db{ name };host=$db{ host }:$db{ port }", $db{ user }, $db{ pass });
my $sql = undef;

my $SIZE_TO_LOAD = 100;
my $unamelist = './dic/names.dic'; # user names dictionary
my $modalitylist = './dic/modality.dic';

my @mods = loadModality ();
my %c = (); # generated content
my %uids = ();

### Gen functions ###
sub genNames {
	open (HANDLE, '<', $unamelist) or die "Could not open users dictionary $unamelist";
	my @dic = <HANDLE>;
	close HANDLE;

	my %data = ();
	for (my $i = 0; $i < $SIZE_TO_LOAD; $i++) {
		my $username = $dic[ int (rand (scalar @dic)) ];
		$username =~ s/^\s+//g;
		$username =~ s/\s+$//g;
		$data{ $username } = 1 if !exists $data{ $username };
	}

	return \%data;
}

sub genModality {
	return $mods[ int (rand (scalar @mods)) ];
}

sub loadModality {
	open (HANDLE, '<', $modalitylist) or die "Could not open modality list $modalitylist";
	chomp (my @modalitylist = <HANDLE>);
	close HANDLE;
}

sub genId {
	my $length = shift || 16;
	my @arr = ("A".."Z", "a".."z", 0..9);

	srand ();
	$length = int (rand ($length) + 2); # minimal length is 2

	return join ('', @arr[ map{ rand @arr }(1..$length) ]);
}

# https://www.medicalconnections.co.uk/kb/UID_Rules
sub genUid {
	my $pre = "1.2.826.0.1.3680043.9.4554"; # Petrovich UID
	my $post = undef;

	do {
		$post = int (rand (100000000));
	} while (exists $uids{ "$pre.$post" });
	$uids{ "$pre.$post" } = 1; # adding new uid

	return "$pre.$post";
}

sub genTime {
	srand ();
	
	my $hh = sprintf ("%02d", int (rand (24)) + 1);
	my $mm = sprintf ("%02d", int (rand (60)));
	my $ss = sprintf ("%02d", int (rand (60)));
	my $frac = sprintf ("%04d", int (rand (1000)));

	return "$hh$mm$ss.$frac";
}

### Main 

%c = %{ genNames () };
my $iter = 0;
foreach my $username (keys %c) {
	srand ();
	my @arr = ();

	# Patient level
	my $ut = int (rand (900000000)) + 461928725; # unix time for birth date <= 30 yo max

	$sql = qq|INSERT INTO patient (PatientID, PatientName, PatientSex, PatientBirthDate) VALUES (?, ?, ?, ?)|;
	$c{ $username } = {
		patient => {
			PatientID	=> genId (64),
			PatientName	=> $username,
			PatientSex	=> (int (rand (2))) ? 'Male' : 'Female',
			PatientBirthDate	=> localtime($ut)->strftime ("%Y%m%d")
		}
	};
	$dbh->do ($sql, undef, map { $c{ $username }{ patient }{ $_ } } qw|PatientID PatientName PatientSex PatientBirthDate|) or die "$!";
	my $patient_id = $dbh->{ q|mysql_insertid| }; # auto inc

	# Study level
	$ut = int (rand (900000000)) + 461928725; # re-generated unix time

	$sql = qq|INSERT INTO study (patient_id, StudyID, StudyInstanceUID, AccessionNumber, StudyDate, StudyTime) VALUES (?, ?, ?, ?, ?, ?)|;
	$c{ $username }{ study } = {
		patient_id	=> $patient_id,
		StudyID		=> genId (16),
		StudyInstanceUID	=> genUid (),
		AccessionNumber	=> 'FUJI95714',
		StudyDate	=> localtime($ut)->strftime ("%Y%m%d"),
		StudyTime	=> genTime ()
	};
	$dbh->do ($sql, undef, map { $c{ $username }{ study }{ $_ } } qw|patient_id StudyID StudyInstanceUID AccessionNumber StudyDate StudyTime|) or die "$!";
	my $study_id = $dbh->{ q|mysql_insertid| }; # auto inc

	# Series level
	$sql = qq|INSERT INTO series (study_id, SeriesInstanceUID, SeriesDate, SeriesTime, SeriesNumber, Modality) VALUES (?, ?, ?, ?, ?, ?)|;
	$c{ $username }{ series } = {
		study_id		=> $study_id,
		SeriesInstanceUID	=> genUid (),
		SeriesDate	=> $c{ $username }{ study }{ StudyDate },
		SeriesTime	=> $c{ $username }{ study }{ StudyTime },
		SeriesNumber=> $iter,
		Modality		=> genModality ()
	};
	$dbh->do ($sql, undef, map { $c{ $username }{ series }{ $_ } } qw|study_id SeriesInstanceUID SeriesDate SeriesTime SeriesNumber Modality|) or die "$!";
	my $series_id = $dbh->{ q|mysql_insertid| };

	# Object
	$sql = qq|INSERT INTO object (series_id, SOPInstanceUID, InstanceCreationDate, InstanceCreationTime, InstanceNumber) VALUES (?, ?, ?, ?, ?)|;
	$c{ $username }{ object } = {
		series_id	=> $series_id,
		SOPInstanceUID	=> genUid (),
		InstanceCreationDate	=> $c{ $username }{ study }{ StudyDate },
		InstanceCreationTime	=> $c{ $username }{ study }{ StudyTime },
		InstanceNumber	=> 1 # first image only (one at all)
	};
	$dbh->do ($sql, undef, map { $c{ $username }{ object }{ $_ } } qw|series_id SOPInstanceUID InstanceCreationDate InstanceCreationTime InstanceNumber| ) or die "$!";

	$iter++;
}

print Dumper (\%c);
exit 0;
