#!/usr/bin/perl

use warnings;
use strict;


my $scale = 0.4;
my $offset = 0;
my $suffix = "";

my @vertices;
my @faces;

my $material = 0;
my $lastmaterial = 0;
my %materials = ();

while(<>) {
        if( $_ =~ /usemtl ([^ ]+)/ ) {
            if(defined $materials{$1}) {
                $material = $materials{$1};
            }
            else {
                $material = $lastmaterial;
                $materials{$1} = $material;
                $lastmaterial++;
            }
        }
        
	if( $_ =~ /v ([^ ]+) ([^ ]+) ([^ ]+)/ ) {
		push @vertices, [$1, $2, $3];
	}

        # Assume normals are per face,
	if( $_ =~ /f ([0-9]+)\/[0-9]*\/([0-9]+) ([0-9]+)\/[0-9]*\/([0-9]+) ([0-9]+)\/[0-9]*\/([0-9]+)/ ) {
		push @faces, [$1, $3, $5, $2];
	}

# 	if( $_ =~ /vn ([^ ]+) ([^ ]+) ([^ ]+)/ ) {
#                 my $normalid = "$1__$2__$3__";
# 		push @normals, [$1, $2, $3];
# 	}
}

print "#define numVertices$suffix " . scalar @vertices . "\n";
# print "#define numNormals$suffix " . scalar @normals . "\n";
print "#define numFaces$suffix " . scalar @faces . "\n\n";

print "#include \"Rasterize.h\"\n\n";

print "const init_vertex_t vertices" . $suffix . "[] = {\n";
foreach(@vertices) {
	my @vertex = @{$_};
	print "\t{ F(" . $vertex[0]*$scale .
		 "), F(" . $vertex[1]*$scale .
		 "), F(" . $vertex[2]*$scale . ") }, \n";
}
print "};\n\n";

# print "const init_vertex_t normals" . $suffix . "[] = {\n";
# foreach(@normals) {
#         my @normal = @{$_};
#         print "\t{ F(" . $normal[0]*1.0 .
#                  "), F(" . $normal[1]*1.0 .
#                  "), F(" . $normal[2]*1.0 . ") }, \n";
# }
# print "};\n\n";

print "const index_triangle_t faces" . $suffix . "[] = {\n";
foreach(@faces) {
	my @face = @{$_};
	print "\t{" . ($face[0] - 1 + $offset) . ", " .
                ($face[1] - 1 + $offset) . ", " .
                ($face[2] - 1 + $offset) . ", " .
                ($face[3] - 1 + $offset) . "},\n";
}
print "};\n";