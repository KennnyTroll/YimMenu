# Lecture du fichier source
with open('source.txt', 'r') as source_file:
    lines = source_file.readlines()

# Traitement des lignes pour extraire les paires clé-valeur
pairs = []
for line in lines:
    line = line.strip()  # Supprimer les espaces et les sauts de ligne supplémentaires
    if line:  # Ignorer les lignes vides
        key, value = line.split('=')  # Diviser la ligne en clé et valeur
        key = key.strip()  # Supprimer les espaces autour de la clé
        value = int(value.split(',')[0].strip())  # Supprimer la virgule et convertir la valeur en entier
        pairs.append((key, value))  # Ajouter la paire à la liste

# Écriture des paires dans le fichier de destination
with open('destination.txt', 'w') as dest_file:
    for pair in pairs:
        dest_file.write('{{ "{0}", {1} }},\n'.format(pair[0], pair[1]))