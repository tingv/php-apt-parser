<?php
    //////////////////////////////////////////////////////////////////
    // PHP Apt-file parser, Version 1.2
    //
    // The following PHP functions parse Debian APT-repository files
    // (Packages.gz, Sources.gz) and generate an XHTML summary of them.
    //
    // Written by Jarno Elonen in 2003, 2004
    // Released into Public Domain without any warranties.
    //
    // Usage:
    //
    //   include_once("parse-apt-files.inc");
    //   parse_and_list( "Packages.gz", "Sources.gz" );
    //
    // ... or if the repository consist of multiple Packages files:
    //
    //   include_once("parse-apt-files.inc");
    //   parse_and_list( Array("i386/Packages.gz", "powerpc/Packages.gz"), "Sources.gz" );
    //
    //
    // Changelog
    //
    //    2004-08-09, Jarno Elonen <elonen@debian.org>:
    //        + version 1.1
    //        + added support for multiple architectures (thanks to  Djoumé Salvetti)
    //        + added support for multiple versions of the same package
    //        + changed link_aware_htmlize() to accept all sorts of URLs, not just HTTP
    //        + simplified the code here and there
    //
    //    2004-10-17, Jarno Elonen <elonen@debian.org>:
    //        + version 1.2
    //        + search for .changes file and print a link if possible
    //        + support for non-compressed Packages and Sources files
    //
    //////////////////////////////////////////////////////////////////

    // Parse Packages.gz or Sources.gz file and return an array of format
    // Array( "<pkg-name>", Array( "<Field-name>" => "<value>", ... ))
    // Note that the result may contain several versions of the same package
    function parse_packages( $filename )
    {
        $lines = (strpos( $filename, '.gz' ) !== False)
          ? gzfile($filename) : file($filename);

        $i = -1;
        $curtag = false;
        $newtag = true;
        $value = "";
        $res = Array();
        foreach( $lines as $line )
        {
            $matches = Array();
            if ( preg_match( "/^([^ ]*):( )?(.*)$/", $line, $matches ))
            {
                if ( $curtag !== false && $i >= 0 )
                    $res[$i][1][$curtag] = $value;

                $curtag = $matches[1];
                $value = $matches[3];

                if ( $curtag == "Package" )
                    $res[++$i] = Array(0 => $value, 1 => Array());
                $newtag = true;
            }
            else
            {
                if ( trim($line) !== "." )
                {
                    if ( $newtag || substr( 0,2 ) == "  " )
                        $value .= "\n";
                    else
                        $value .= " ";
                    $value .= trim( $line );
                }
                else $value .= "\n\n";
                $newtag = false;
            }
        }
        if ( $curtag !== false && $i >= 0 )
            $res[$i][1][$curtag] = $value;

        return $res;
    }

    // Convert given byte-size to more human readable format
    function human_format_size( $bytes )
    {
        $formats = Array("%d Bytes", "%.1f KB", "%.1f MB", "%.1f GB", "%.1f TB");
        $logsize = min((int)(log($bytes)/log(1024)), count($formats)-1);
        return sprintf( $formats[$logsize], $bytes/pow(1024, $logsize));
    }

    // Extract URLs from text, make them links and output
    // the whole thing as HTML
    function link_aware_htmlize( $txt )
    {
        $res = "";
        $linkpattern = '!([a-z]+://[^ $]+)!is';
        $frags = preg_split( $linkpattern, $txt, -1, PREG_SPLIT_DELIM_CAPTURE);
        foreach( $frags as $f )
        {
            if ( preg_match( $linkpattern, $f ))
                $res .= '<a href="' . htmlentities($f) . '">' . htmlentities($f) . '</a>';
            else
                $res .= htmlentities( $f );
        }
        return str_replace( "\n", "<br/>", $res );
    }

    // Parse given files and write out a nice summary in HTML
    function parse_and_list( $packagesgzfiles, $sourcesgzfile )
    {
        $sources_temp = parse_packages( $sourcesgzfile );

    // Rebuild sources array
        $sources = Array();
        while( list($i, $p) = each( $sources_temp ))
            $sources[$p[0]] = $p[1];

        if (!is_array($packagesgzfiles))
            $packagesgzfiles = Array($packagesgzfiles);

        // Merge different architectures into one $packages array
        $packages = Array();
        foreach( $packagesgzfiles as $f )
        {
            $pkgs = parse_packages( $f );
            while( list($i, $p) = each( $pkgs ))
            {
                $name_and_ver = $p[0] . '-' . $p[1]['Version'];
                if (!isset($packages[$name_and_ver]))
                    $packages[$name_and_ver] = Array();
                $packages[$name_and_ver][$p[1]['Architecture']] = $p;
            }
        }

        // Walk through the packages and print out the info
        $first = true;
        while( list($name_and_ver, $archs) = each( $packages ))
        {
            print '<div>';

            if ( !$first )
                print "<br/>\n";
            $first = false;

            // use the first arch for description etc.
            ksort( $archs );
            list($pkg, $fields) = reset($archs);

            $srcfields = false;
            if ( isset( $fields["Source"] ) &&
                 isset( $sources[$fields["Source"]] ))
                    $srcfields = $sources[$fields["Source"]];
            else if ( isset( $sources[$pkg] ))
                $srcfields = $sources[$pkg];

            print '<b>' . htmlentities( $fields["Package"] ) . '</b> '.
                  '<small>(' . htmlentities( $fields["Version"] ) . ')</small>' . "\n";

            if ( strlen(trim($fields["Description"])))
                print ' - ' . htmlentities( preg_replace( '/[\n].*/', '',
                    $fields["Description"] ));
            print "<br/><small>\n";

            $archlinks = Array();
            while( list($arch_name, list(, $arch_fields)) = each( $archs ))
                $archlinks[] = '<a href="' .
                    htmlentities( $arch_fields["Filename"] ) . '">' .
                    htmlentities( $arch_name ) . ' (' .
                    human_format_size( $arch_fields["Size"]) . ")</a>\n";
            print 'Binary for arch ' . join(', ', $archlinks);

            if ( isset( $srcfields["Directory"] ))
            {
                print '| <a href="' .
                    htmlentities( $srcfields["Directory"] ) .
                    '">Source dir</a>' . "\n";

                // Try to make a changes file link. This adds quite a bit
                // of disk access so if your server is very busy, you might
                // want to comment this out.
                if ($dhandle = opendir( $srcfields["Directory"] ))
                {
                    $chfile = False;
                    while (false !== ($file = readdir($dhandle)))
                        if (strpos($file, $srcfields["Package"] . '_' . $srcfields["Version"] . '_') !== False &&
                            strpos($file, '.changes') !== False )
                        {
                          print '| <a href="' . htmlentities( $srcfields["Directory"] . '/' . $file ) .
                                '">Changes</a>' . "\n";
                          break;
                        }
                    closedir($dhandle);
                }
            }

            print "<br/><br/></small></div>\n" .
                  "<blockquote><div><small>\n" .
                  link_aware_htmlize(
                      preg_replace( '/^[^\n]*\n/', '',
                          $fields["Description"] )) .
                   "\n</small></div></blockquote>\n\n\n";
        }
    }
?>
