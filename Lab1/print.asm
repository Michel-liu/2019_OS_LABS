data SEGMENT
    msg DB 'Hello World!$'
data ENDS
code SEGMENT
    ASSUME CS:code,DS:data
start:
    MOV AX,DATA
    MOV DS,AX
    LEA dx,msg
    MOV AH,9H
    INT 21H
    MOV AH,4CH
    INT 21H
code ENDS
END start