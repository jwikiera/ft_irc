if [ "$(uname)" = "Linux" ]; then
    ngircd -n --config ngircd.conf
else
    /Users/$USER/Applications/homebrew/Cellar/ngircd/26.1/sbin/ngircd -n --config ngircd.conf
fi
