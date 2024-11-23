# Crear un archivo binario con separadores correctos
with open('test_case_FINAL.bin', 'wb') as f:
    f.write(b'Client Alice\x022024-11-20 01:02:03\x02Hi\x04')
    f.write(b'Agent Bob\x022024-11-20 01:03:03\x02My name is Peter, How can I help you?\x04')
    f.write(b'Client Alice\x022024-11-20 01:04:03\x02I bought a computer from you and when I opened the box the screen was broken and I am also making this message extremely long so that it fails the standards why why why why why whywhy why whywhy why whywhy why whywhy why why\x04')
    f.write(b'Agent Bob\x02this is no timestamp\x02Oh, so sorry to hear that, I am going to help you get a new computer. I am going to need the purchase information\x04')
    f.write(b'Client Nicolas\x022024-11-20 01:06:03\x02The name has forbidden chars!!!\x04')
    f.write(b'Agent Bob\x022024-11-20 01:07:03\x02The message has forbidden chars!!!\x04')
print("Archivo binario creado exitosamente.")