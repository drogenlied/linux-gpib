(define (println . args) (for-each display args) (newline))

(define gpib:handle '())

(define (gpib:init)
  (catch 'misc-error
         (lambda ()
           (if (not (feature? 'gpib))
               (begin
                 (set! gpib:handle (dynamic-link "libgpib-guile.so"))
                 (dynamic-call "gpib_init" gpib:handle))))
	 (lambda args #f)))

(define (gpib:halt)
  (if (dynamic-object? gpib:handle)
      (begin (dynamic-unlink gpib:handle)
             (set! gpib:handle '()))))

(define (gpib:open index pad sad timeout eoi eos)
  (ibdev index pad sad timeout eoi eos))

(define (gpib:command fd list)
  (let* ((clist '()))
    (for-each (lambda (c)
		(set! clist (cons (integer->char c) clist)))
	      list)
    (ibcmd fd (list->string (reverse clist)))))

(define (gpib:write fd string)
  (ibwrt fd string))

(define (gpib:read fd bytes)
  (ibrd fd bytes))

(define (gpib:find name)
  (ibfind name))

(define (gpib:remote-enable fd enable)
  (ibsre fd enable))

(define (gpib:interface-clear fd)
  (ibsic fd))

(define (gpib:device-clear fd)
  (ibclr fd))

(define (gpib:reset fd)
  (ibonl fd 1))

(define (gpib:close fd)
  (ibonl fd 0))

(define (gpib:wait fd status)
  (ibwait fd status))

(define (gpib:serial-poll fd)
  (ibrsp fd))

(define (gpib:trigger fd)
  (ibtrg fd))

(define (gpib:request-service fd service)
  (ibrsv fd service))

(define (gpib:error-code)
  (iberr))

(define (gpib:counter)
  (ibcnt))

(define (gpib:error)
  (let* ((error (iberr)))
    (cond
     ((equal? error EDVR) "<OS Error>")
     ((equal? error ECIC) "<Not CIC>")
     ((equal? error ENOL) "<No Listener>")
     ((equal? error EADR) "<Adress Error>")
     ((equal? error ECIC) "<Invalid Argument>")
     ((equal? error ESAC) "<No Sys Ctrlr>")
     ((equal? error EABO) "<Operation Aborted>")
     ((equal? error ENEB) "<No Gpib Board>")
     ((equal? error EOIP) "<Async I/O in prg>")
     ((equal? error ECAP) "<No Capability>")
     ((equal? error EFSO) "<File sys. error>")
     ((equal? error EBUS) "<Command error>")
     ((equal? error ESTB) "<Status byte lost>")
     ((equal? error ESRQ) "<SRQ stuck on>")
     ((equal? error ETAB) "<Device Table Overflow>"))))

(export 

 ;; public Gpib procedures
 gpib:init
 gpib:halt
 gpib:open
 gpib:command
 gpib:write
 gpib:read
 gpib:find
 gpib:remote-enable
 gpib:interface-clear
 gpib:device-clear
 gpib:close
 gpib:reset
 gpib:wait
 gpib:serial-poll
 gpib:trigger
 gpib:request-service
 gpib:error-code
 gpib:counter
 gpib:error

 ;; status byte
 DCAS DTAS LACS TACS ATN CIC REM LOK CMPL EVENT SPOLL RQS SRQI END TIMO ERR

 ;; public Gpib commands
 GTL SDC PPC GET TCT LLO DCL PPU SPE SPD UNL UNT PPD

 ;; timeout constants
 TNONE T10us T30us T100us T300us T1ms T3ms T10ms T30ms T100ms T300ms T1s
 T3s   T10s  T30s  T100s  T300s  T1000s
)
