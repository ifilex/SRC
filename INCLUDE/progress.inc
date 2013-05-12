externdef	progress_dobj:word
externdef	progress_size:dword

progress_open	proto dist pascal :dword, :dword
progress_close	proto dist
progress_set	proto dist pascal :dword, :dword, :dword
progress_update	proto dist pascal :dword
