<?
$domDocument = new DOMDocument();
$domDocument->load("instructions.xml");

$head = $domDocument->getElementsByTagName("head")->item(0);
print "$head->textContent\n";

$definitions = $domDocument->getElementsByTagName("definitions")->item(0);
foreach ($definitions->childNodes as $def) {
  if ($def->hasChildNodes()) {
    $defName;
    $defVal;
    $defComment;
    foreach ($def->childNodes as $node) {
      switch ($node->nodeName) {
        case "name":
          $defName = $node->textContent;
          break;
        case "value":
          $defVal = $node->textContent;
          break;
        case "comment":
          $defComment = $node->textContent;
          break;
      }
    }
    if ($defComment) {
      print "/* $defComment */\n";
    }
    print "#define $defName $defVal\n";
    unset ($defName);
    unset ($defVal);
    unset ($defComment);
  }
}

$args = $domDocument->getElementsByTagName("args")->item(0);
print "\nenum TypeOfArgument {\n\n";
foreach ($args->childNodes as $instr) {
  if ($instr->hasChildNodes()) {
    $instrName;
    $instrVal;
    foreach ($instr->childNodes as $node) {
      switch ($node->nodeName) {
        case "name":
          $instrName = $node->textContent;
          break;
        case "value":
          $instrVal = $node->textContent;
          break;
      }
    }
    print "  $instrName". ($instrVal ? " = $instrVal" : "") .",\n";
    unset ($instrName);
    unset ($instrVal);
  }
}
print "};\n";

$cpuInstrs = $domDocument->getElementsByTagName("cpu")->item(0);
print "\nenum StdInstructions {\n\n";
foreach ($cpuInstrs->childNodes as $instr) {
  if ($instr->hasChildNodes()) {
    $instrName;
    $instrVal;
    $instrCode;
    foreach ($instr->childNodes as $node) {
      switch ($node->nodeName) {
        case "name":
          $instrName = $node->textContent;
          break;
        case "value":
          $instrVal = $node->textContent;
          break;
        case "code":
          $instrCode = $node->textContent;
          break;
      }
    }
    print "  $instrName". ($instrVal ? " = $instrVal" : "") .",\n";
    unset ($instrName);
    unset ($instrVal);
    unset ($instrCode);
  }
}
print "};\n";

$fpuInstrs = $domDocument->getElementsByTagName("fpu")->item(0);
print "\nenum FloatIstructions {\n\n";
foreach ($fpuInstrs->childNodes as $instr) {
  if ($instr->hasChildNodes()) {
    $instrName;
    $instrVal;
    $instrCode;
    foreach ($instr->childNodes as $node) {
      switch ($node->nodeName) {
        case "name":
          $instrName = $node->textContent;
          break;
        case "value":
          $instrVal = $node->textContent;
          break;
        case "code":
          $instrCode = $node->textContent;
          break;
      }
    }
    print "  $instrName". ($instrVal ? " = $instrVal" : "") .",\n";
    unset ($instrName);
    unset ($instrVal);
    unset ($instrCode);
  }
}
print "};\n";

$tail = $domDocument->getElementsByTagName("tail")->item(0);
print "$tail->textContent\n";

?>
