# %%
def cuenta(palabra, palabras, izq, der, contador=0):
    """Simula la función cuenta del ensamblador, implementando búsqueda binaria"""
    # Incrementar el contador con cada llamada
    contador += 1
    
    # Verificar que el contador no exceda 11 (como en la bomba)
    if contador > 11:
        return -1  # Simulando explosión
    
    # Calcular medio usando la lógica del ensamblador
    medio = (izq ^ der) >> 1
    medio += (izq & der)
    
    # Comparar la palabra con la palabra en la posición medio
    comparacion = (palabra == palabras[medio])
    
    # Si la encuentra, retornar el contador
    if comparacion:
        return contador
    
    # Lógica de búsqueda binaria similar a la implementada en cuenta
    if palabra < palabras[medio] and izq < medio:
        return cuenta(palabra, palabras, izq, medio-1, contador)
    elif palabra > palabras[medio] and medio < der:
        return cuenta(palabra, palabras, medio+1, der, contador)
    
    # No encontró la palabra
    return -1

# Leer las palabras del archivo
with open("palabras.txt", "r") as f:
    palabras = [line.strip() for line in f.readlines()]

# Verificar cada palabra
resultados = []
for palabra in palabras:
    # Evitar líneas vacías o comentarios
    if not palabra or palabra.startswith("//"):
        continue
        
    # Calcular cuántos pasos toma encontrarla
    pasos = cuenta(palabra, palabras, 0, len(palabras)-1)
    
    # Si toma MÁS DE 6 pasos y es encontrada
    if pasos > 6:
        resultados.append((palabra, pasos))

# Mostrar resultados
print(f"Encontradas {len(resultados)} palabras válidas:")
for palabra, pasos in resultados:
    print(f"'{palabra} {pasos}'")

# También mostrar algunas estadísticas
if resultados:
    min_pasos = min(p for _, p in resultados)
    print(f"\nPalabras con el mínimo número de pasos ({min_pasos}):")
    for palabra, pasos in resultados:
        if pasos == min_pasos:
            print(f"'{palabra} {pasos}'")

# Modificar el script para depurar específicamente la palabra que intentaste
print("Depuración para palabras específicas:")
for test_palabra in ["agringarse", "engringarse"]:
    if test_palabra in palabras:
        pasos = cuenta(test_palabra, palabras, 0, len(palabras)-1)
        print(f"Palabra '{test_palabra}' encontrada: {pasos} pasos")
    else:
        print(f"Palabra '{test_palabra}' NO encontrada en el archivo")
# %%
