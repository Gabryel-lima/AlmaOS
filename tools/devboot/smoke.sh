#!/bin/sh
# smoke.sh - Roteiro de fumaca do kernel do AlmaOS sob QEMU.
#
# Sobe build/devboot.img sem tela, digita comandos no shell pela COM1 e
# confere se as respostas esperadas aparecem na saida serial. Serve como o
# "roda de verdade?" automatizavel do kernel, sem depender do OpenWatcom nem
# de alguem olhando o console.
#
# Uso: make devboot && tools/devboot/smoke.sh [imagem]

set -eu

IMAGE="${1:-build/devboot.img}"
LOG="$(mktemp)"
trap 'rm -f "$LOG"' EXIT

if [ ! -f "$IMAGE" ]; then
    echo "smoke: imagem nao encontrada: $IMAGE (rode 'make devboot')" >&2
    exit 1
fi

# Os comandos vao pela serial; o kernel trata CR como Enter. A pausa nao e
# decorativa: `serial_init()` limpa o FIFO de recepcao, entao qualquer byte
# que chegue antes do kernel subir e descartado. Esperar o shell chegar ao
# prompt e o que torna o roteiro deterministico.
#
# `gfxdemo ... still` desenha um unico quadro em angulo fixo e le o
# framebuffer do hardware de volta — e o que torna o caminho grafico
# verificavel sem tela. As pausas depois dele sao generosas porque a
# rasterizacao por software sob emulacao nao e rapida.
#
# `timeout` mata o QEMU: o shell e um laco infinito e nunca sai sozinho.
{
    sleep 3
    printf 'help\rmem\rticks\rmode\r'
    sleep 2
    printf 'gfxdemo 320x200x32 still\r'
    sleep 8
    printf 'gfxdemo vga13 still\r'
    sleep 8
} | timeout 60 qemu-system-i386 \
    -drive "file=$IMAGE,format=raw,if=floppy" \
    -boot a -display none -no-reboot \
    -serial stdio > "$LOG" 2>&1 || true

fail=0
check() {
    if grep -q "$1" "$LOG"; then
        echo "  ok   $2"
    else
        echo "  FAIL $2 (esperava /$1/)"
        fail=1
    fi
}

echo "smoke: verificando a saida serial de $IMAGE"
check 'AlmaOS kernel inicializando'   'kernel iniciou'
check '\[OK\] IDT carregada'          'IDT instalada'
check '\[OK\] PIC remapeado'          'PIC remapeado'
check '\[OK\] PIT configurado'        'PIT programado'
check '\[OK\] Log serial COM1'        'COM1 detectada'
check 'AlmaOS> '                      'shell no prompt'
check 'Comandos disponiveis'          'comando help respondeu'
check 'Mapa de memoria: '             'comando mem respondeu'
check 'System ticks: '                'comando ticks respondeu'
check '\[OK\] Trampolim de modo real'  'trampolim de modo real pronto'
check '\[OK\] FPU x87'                 'FPU habilitada'
check '\[OK\] Memoria alta'            'alocador fisico encontrou RAM alta'
check 'Modo atual: texto'             'comando mode respondeu'

# O cubo do gfxdemo tem tres faces visiveis neste angulo. Exigir mais de uma
# cor e o que separa "o z-buffer discriminou profundidade" de "a ultima face
# pintou tudo por cima"; exigir cobertura e o que separa "desenhou" de
# "limpou a tela e foi embora".
check 'gfxdemo: fundo=0x00101830 cobertos=4953 cores=6' 'gfx desenhou o cubo em VBE 32bpp'
check 'gfxdemo: fundo=0x00000000 cobertos=4953 cores=3' 'gfx desenhou o cubo em VGA 13h'
check 'de volta ao modo texto'         'voltou do modo grafico'

if [ "$fail" -ne 0 ]; then
    echo "smoke: FALHOU. Saida serial completa:" >&2
    cat "$LOG" >&2
    exit 1
fi

echo "smoke: tudo passou"
