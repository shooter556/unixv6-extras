if $1x != x goto loop
echo Usage $0 name ...
echo "where:	name	 = directory containing old files"
echo	"	name.slb = is name of slup library"
echo	"	name.mods= is complete set of modifications"
exit
: loop
echo $1:
chdir $1
: slup ../$1.slb -n *
slup ../$1.slb -n
slup ../$1.slb -i [!-g]*
slup ../$1.slb -i [h-z]*
chdir ..
slup $1 -uv <$1.mods
shift
if $1x != x goto loop
