	.arch armv8-a
	.file	"main.c"
	.text
	.section	.rodata
	.align	3
.LC0:
	.string	"Client disconnected"
	.align	3
.LC1:
	.string	"recv failed"
	.text
	.align	2
	.global	client_handler
	.type	client_handler, %function
client_handler:
.LFB6:
	.cfi_startproc
	sub	sp, sp, #1072
	.cfi_def_cfa_offset 1072
	stp	x29, x30, [sp]
	.cfi_offset 29, -1072
	.cfi_offset 30, -1064
	mov	x29, sp
	str	x0, [sp, 24]
	adrp	x0, :got:__stack_chk_guard
	ldr	x0, [x0, #:got_lo12:__stack_chk_guard]
	ldr	x1, [x0]
	str	x1, [sp, 1064]
	mov	x1, 0
	ldr	x0, [sp, 24]
	ldr	w0, [x0]
	str	w0, [sp, 32]
	b	.L2
.L3:
	add	x0, sp, 40
	bl	strlen
	mov	x1, x0
	add	x0, sp, 40
	mov	x2, x1
	mov	x1, x0
	ldr	w0, [sp, 32]
	bl	write
	add	x0, sp, 40
	mov	x2, 1024
	mov	w1, 0
	bl	memset
.L2:
	add	x0, sp, 40
	mov	w3, 0
	mov	x2, 1024
	mov	x1, x0
	ldr	w0, [sp, 32]
	bl	recv
	str	w0, [sp, 36]
	ldr	w0, [sp, 36]
	cmp	w0, 0
	bgt	.L3
	ldr	w0, [sp, 36]
	cmp	w0, 0
	bne	.L4
	adrp	x0, .LC0
	add	x0, x0, :lo12:.LC0
	bl	puts
	adrp	x0, :got:stdout
	ldr	x0, [x0, #:got_lo12:stdout]
	ldr	x0, [x0]
	bl	fflush
	b	.L5
.L4:
	ldr	w0, [sp, 36]
	cmn	w0, #1
	bne	.L5
	adrp	x0, .LC1
	add	x0, x0, :lo12:.LC1
	bl	perror
.L5:
	ldr	w0, [sp, 32]
	bl	close
	ldr	x0, [sp, 24]
	bl	free
	mov	x0, 0
	mov	x1, x0
	adrp	x0, :got:__stack_chk_guard
	ldr	x0, [x0, #:got_lo12:__stack_chk_guard]
	ldr	x3, [sp, 1064]
	ldr	x2, [x0]
	subs	x3, x3, x2
	mov	x2, 0
	beq	.L7
	bl	__stack_chk_fail
.L7:
	mov	x0, x1
	ldp	x29, x30, [sp]
	add	sp, sp, 1072
	.cfi_restore 29
	.cfi_restore 30
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE6:
	.size	client_handler, .-client_handler
	.section	.rodata
	.align	3
.LC2:
	.string	"Error creating socket"
	.align	3
.LC3:
	.string	"Bind failed"
	.align	3
.LC4:
	.string	"epoll_create1"
	.align	3
.LC5:
	.string	"epoll_ctl: server_socket"
	.align	3
.LC6:
	.string	"accept"
	.align	3
.LC7:
	.string	"epoll_ctl: client_socket"
	.text
	.align	2
	.global	main
	.type	main, %function
main:
.LFB7:
	.cfi_startproc
	sub	sp, sp, #1216
	.cfi_def_cfa_offset 1216
	stp	x29, x30, [sp]
	.cfi_offset 29, -1216
	.cfi_offset 30, -1208
	mov	x29, sp
	adrp	x0, :got:__stack_chk_guard
	ldr	x0, [x0, #:got_lo12:__stack_chk_guard]
	ldr	x1, [x0]
	str	x1, [sp, 1208]
	mov	x1, 0
	mov	w0, 16
	str	w0, [sp, 24]
	mov	w2, 0
	mov	w1, 1
	mov	w0, 2
	bl	socket
	str	w0, [sp, 32]
	ldr	w0, [sp, 32]
	cmn	w0, #1
	bne	.L9
	adrp	x0, .LC2
	add	x0, x0, :lo12:.LC2
	bl	perror
	mov	w0, -1
	b	.L19
.L9:
	mov	w0, 2
	strh	w0, [sp, 72]
	str	wzr, [sp, 76]
	mov	w0, 8080
	bl	htons
	and	w0, w0, 65535
	strh	w0, [sp, 74]
	add	x0, sp, 72
	mov	w2, 16
	mov	x1, x0
	ldr	w0, [sp, 32]
	bl	bind
	cmp	w0, 0
	bge	.L11
	adrp	x0, .LC3
	add	x0, x0, :lo12:.LC3
	bl	perror
	mov	w0, -1
	b	.L19
.L11:
	mov	w1, 5
	ldr	w0, [sp, 32]
	bl	listen
	mov	w0, 0
	bl	epoll_create1
	str	w0, [sp, 36]
	ldr	w0, [sp, 36]
	cmn	w0, #1
	bne	.L12
	adrp	x0, .LC4
	add	x0, x0, :lo12:.LC4
	bl	perror
	mov	w0, 1
	b	.L19
.L12:
	ldr	w0, [sp, 32]
	str	w0, [sp, 64]
	mov	w0, -2147483647
	str	w0, [sp, 56]
	add	x0, sp, 56
	mov	x3, x0
	ldr	w2, [sp, 32]
	mov	w1, 1
	ldr	w0, [sp, 36]
	bl	epoll_ctl
	cmn	w0, #1
	bne	.L13
	adrp	x0, .LC5
	add	x0, x0, :lo12:.LC5
	bl	perror
	mov	w0, 1
	b	.L19
.L13:
	add	x0, sp, 184
	mov	w3, -1
	mov	w2, 64
	mov	x1, x0
	ldr	w0, [sp, 36]
	bl	epoll_wait
	str	w0, [sp, 40]
	str	wzr, [sp, 28]
	b	.L14
.L18:
	ldrsw	x0, [sp, 28]
	lsl	x0, x0, 4
	add	x1, sp, 192
	ldr	w0, [x1, x0]
	ldr	w1, [sp, 32]
	cmp	w1, w0
	bne	.L15
	add	x1, sp, 24
	add	x0, sp, 88
	mov	x2, x1
	mov	x1, x0
	ldr	w0, [sp, 32]
	bl	accept
	str	w0, [sp, 44]
	ldr	w0, [sp, 44]
	cmn	w0, #1
	bne	.L16
	adrp	x0, .LC6
	add	x0, x0, :lo12:.LC6
	bl	perror
	b	.L17
.L16:
	ldr	w0, [sp, 44]
	str	w0, [sp, 64]
	mov	w0, -2147483647
	str	w0, [sp, 56]
	add	x0, sp, 56
	mov	x3, x0
	ldr	w2, [sp, 44]
	mov	w1, 1
	ldr	w0, [sp, 36]
	bl	epoll_ctl
	cmn	w0, #1
	bne	.L17
	adrp	x0, .LC7
	add	x0, x0, :lo12:.LC7
	bl	perror
	mov	w0, 1
	b	.L19
.L15:
	mov	x0, 4
	bl	malloc
	str	x0, [sp, 48]
	ldrsw	x0, [sp, 28]
	lsl	x0, x0, 4
	add	x1, sp, 192
	ldr	w1, [x1, x0]
	ldr	x0, [sp, 48]
	str	w1, [x0]
	add	x1, sp, 104
	ldrsw	x0, [sp, 28]
	lsl	x0, x0, 3
	add	x4, x1, x0
	ldr	x3, [sp, 48]
	adrp	x0, client_handler
	add	x2, x0, :lo12:client_handler
	mov	x1, 0
	mov	x0, x4
	bl	pthread_create
.L17:
	ldr	w0, [sp, 28]
	add	w0, w0, 1
	str	w0, [sp, 28]
.L14:
	ldr	w1, [sp, 28]
	ldr	w0, [sp, 40]
	cmp	w1, w0
	blt	.L18
	b	.L13
.L19:
	mov	w1, w0
	adrp	x0, :got:__stack_chk_guard
	ldr	x0, [x0, #:got_lo12:__stack_chk_guard]
	ldr	x3, [sp, 1208]
	ldr	x2, [x0]
	subs	x3, x3, x2
	mov	x2, 0
	beq	.L20
	bl	__stack_chk_fail
.L20:
	mov	w0, w1
	ldp	x29, x30, [sp]
	add	sp, sp, 1216
	.cfi_restore 29
	.cfi_restore 30
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE7:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
