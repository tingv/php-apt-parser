#!/usr/bin/php
<? echo "Content-Type: text/html; charset=UTF-8\n\n"; ?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <meta name="Description" content="Debian packages in my unofficial Debian repository" />
    <link rel="stylesheet" type="text/css" title="Default" href="/packages.css" />
    <title>Debian packages in my unofficial Debian repository</title>
  </head>
  <body>
    <h1>Debian packages in my unofficial Debian repository</h1>
<?
      include_once("parse-apt-files.inc");

			$config = parse_ini_file("/etc/php-apt-parser/config.ini", TRUE);
      $base_url= $config["base_url"];
      unset($config["base_url"]);
      $base_path= $config["base_path"];
      unset($config["base_path"]);

?>
    <p>
      This is where I keep my Debian packages, official and unofficial ones.
    </p>
    <p>
      The repository can be accessed by:
    </p>
    <pre>
      deb <?= $base_url ?> unstable main non-free contrib
      deb-src <?= $base_url ?> unstable main non-free contrib
    </pre>
    <p>
      Of course you may need to change unstable to fit your distribution. Distribution names also work (i.e. sarge, etch, sid).
    </p>
		<?
		foreach($config as $distribution => $keys) {
		 $path = $base_path . $keys["packages_dir"];
		 $arch = $keys["arch"] ? $keys["arch"] : "i386";
		 $sections = explode(",", $keys["sections"]);
		 $changes = $base_path . $keys["changes_dir"];
		 $logs = $base_path . $keys["buildlogs_dir"];
		 $packages = Array();
		 $sources = Array();
		  foreach($sections as $section) {
			 array_push($packages , $path . "/" . $section . "/binary-" . $arch . "/Packages.gz");
			 array_push($sources , $path . "/" . $section . "/source/" . "Sources.gz");
			}
		?>
			<h2>Packages in <?= $distribution ?></h2>
			<?
				parse_and_list($packages, $sources, $changes, $logs);
			?>
		<?
		}
		?>
  </body>
</html>
