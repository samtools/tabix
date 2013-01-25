use ExtUtils::MakeMaker;
WriteMakefile(
			  NAME         => 'Tabix',
			  VERSION_FROM => 'Tabix.pm',
			  LIBS         => ['-L.. -ltabix -lz'],
			  DEFINE       => '-D_FILE_OFFSET_BITS=64 -D_USE_KNETFILE',
			  INC          => '-I..',
			 );
