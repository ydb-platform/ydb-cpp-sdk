<?php
$p = new PDO('odbc:' . $argv[1], '', '', [PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION]);
$s = $p->query("SELECT '" . str_repeat('x',600) . "' AS value");
$r = $s->fetch(PDO::FETCH_LAZY);
for($i=0;$i<3;++$i){$v=$r->value;echo "read $i type=".gettype($v)." length=".(is_string($v)?strlen($v):-1)."\n";}
